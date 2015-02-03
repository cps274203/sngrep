/**************************************************************************
 **
 ** sngrep - SIP Messages flow viewer
 **
 ** Copyright (C) 2013,2014 Ivan Alonso (Kaian)
 ** Copyright (C) 2013,2014 Irontec SL. All rights reserved.
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **
 ****************************************************************************/
/**
 * @file ui_manager.h
 * @author Ivan Alonso [aka Kaian] <kaian@irontec.com>
 *
 * @brief Functions to manage interface panels
 *
 * All sngrep panel pointers are encapsulated into a ui structure that is
 * used to invoke custom functions for creating, destroying, drawing, etc
 * the screens.
 *
 * This sctructure also manages concurrents updates and access to ncurses
 * panel pointers.
 *
 */
#ifndef __SNGREP_UI_MANAGER_H
#define __SNGREP_UI_MANAGER_H

#include <ncurses.h>
#include <panel.h>
#include "sip.h"
#include "group.h"

//! Shorter declaration of ui structure
typedef struct ui ui_t;

/**
 * @brief Panel information structure
 *
 * This struct contains the panel related data, including
 * a pointer to the function that manages its drawing
 */
struct ui
{
    //! Panel Type @see panel_types enum
    int type;
    //! The actual ncurses panel pointer
    PANEL *panel;
    //! Constructor for this panel
    PANEL *
    (*create)();
    //! Destroy current panel
    void
    (*destroy)(PANEL *);
    //! Request the panel to redraw its data
    int
    (*draw)(PANEL*);
    //! Check if the panel request redraw with given msg
    int
    (*redraw_required)(PANEL *, sip_msg_t *);
    //! Handle a custom keybind on this panel
    int
    (*handle_key)(PANEL*, int key);
    //! Show help window for this panel (if any)
    int
    (*help)(PANEL *);

    //! UI lock. Avoid multi-thread update on this UI
    pthread_mutex_t lock;

};

/**
 * @brief Enum for available color pairs
 */
enum sngrep_colors_pairs
{
    CP_DEFAULT = 0,
    CP_CYAN_ON_DEF,
    CP_YELLOW_ON_DEF,
    CP_MAGENTA_ON_DEF,
    CP_GREEN_ON_DEF,
    CP_RED_ON_DEF,
    CP_BLUE_ON_DEF,
    CP_WHITE_ON_DEF,
    CP_DEF_ON_CYAN,
    CP_DEF_ON_BLUE,
    CP_BLACK_ON_CYAN,
};

// Used to configure color pairs only with fg color
#define COLOR_DEFAULT -1

/**
 * @brief Enum for available panel types
 *
 * Mostly used for managing keybindings and offloop ui refresh
 *
 * FIXME Replace this IDs for something more representative
 */
enum panel_types
{
    //! Call List ui screen
    MAIN_PANEL = 0,
    //! Call-Flow ui screen
    DETAILS_PANEL,
    //! Raw SIP messages ui screen
    RAW_PANEL,
    //! Filters panel
    FILTER_PANEL,
    //! Save to pcap panel
    SAVE_PANEL,
    //! Save to txt panel
    SAVE_RAW_PANEL,
    //! Message comprare
    MSG_DIFF_PANEL,
    //! Column selector panel
    COLUMN_SELECT_PANEL,
};

/**
 * @brief Initialize ncurses mode
 *
 * This functions will initialize ncurses mode
 *
 * @returns 0 on ncurses initialization success, 1 otherwise
 */
int
init_interface();

/**
 * @brief Stops ncurses mode
 *
 * This functions will deinitialize ncurse mode
 *
 * @returns 0 on ncurses initialization success, 1 otherwise
 */
int
deinit_interface();

/**
 * @brief Create a panel structure
 *
 * Create a ncurses panel associated to the given ui
 * This function is a small wrapper for panel create function
 *
 * @param ui UI structure
 * @return the ui structure with the panel pointer created
 */
ui_t *
ui_create(ui_t *ui);

/**
 * @brief Destroy a panel structure
 *
 * Removes the panel associatet to the given ui and free
 * its memory. Most part of this task is done in the custom
 * destroy function of the panel.
 *
 * @param ui UI structure
 */
void
ui_destroy(ui_t *ui);

/**
 * @brief Get panel pointer from an ui element
 *
 * Basic getter to get the Ncurses PANEL pointer
 * from ui structure. Use this instead of accessing
 * directly to the pointer.
 *
 * @param ui UI structure
 * @return ncurses panel pointer of given UI
 */
PANEL *
ui_get_panel(ui_t *ui);

/**
 * @brief Check if the given msg makes the UI redraw
 *
 * This function is ivoked every time a new package is readed
 * in online mode. Check if the ui needs to be redrawn with the
 * message to avioid not needed work.
 *
 * @param ui UI structure
 * @param msg las readed message
 * @return 0 in case of redraw required, -1 otherwise
 */
int
ui_redraw_required(ui_t *ui, sip_msg_t *msg);

/**
 * @brief Redrawn current ui
 *
 * This function acts as wrapper to custom ui draw functions
 * with some checks
 *
 * @param ui UI structure
 * @return 0 if ui has been drawn, -1 otherwise
 */
int
ui_draw_panel(ui_t *ui);

/**
 * @brief Show help screen from current UI (if any)
 *
 * This function will display the help screen for given
 * ui if exits.
 * All help screens exits after any character input
 *
 * @param ui UI structure
 */
void
ui_help(ui_t *ui);

/**
 * @brief Handle key inputs on given UI
 *
 * This function will pass the input key sequence
 * to the given UI. This will only happen if the key
 * sequence don't match any of the general keybindings
 *
 * @param ui UI structure
 * @param key keycode sequence of the pressed keys and mods
 */
int
ui_handle_key(ui_t *ui, int key);

/**
 * @brief Find a ui from its pannel pointer
 */
ui_t *
ui_find_by_panel(PANEL *panel);

/**
 * @brief Find a ui form its panel id
 */
ui_t *
ui_find_by_type(int type);

/**
 * @brief Wait for user input.
 *
 * This function manages all user input in all panel types and
 * redraws the panel using its own draw function
 *
 * @param ui the topmost panel ui structure
 */
int
wait_for_input(ui_t *ui);

/**
 * @brief Default handler for keys
 *
 * If ui doesn't handle the given key (ui_handle_key returns -1)
 * then the default handler will be invoked
 *
 * @param ui Current displayed UI structure
 * @param key key pressed by user
 */
int
default_handle_key(ui_t *ui, int key);

/**
 * @brief Draw a box around passed windows
 *
 * Draw a box around passed windows  with two bars
 * (top and bottom) of one line each.
 *
 * FIXME The parameter should be a panel, or ui, but not a window..
 *
 * @param win Window to draw borders on
 */
void
title_foot_box(WINDOW *win);

/**
 * @brief Update topmost panel with the newest readed message
 *
 * This function is invocked asynchronously from the
 * capture thread to notify a new message of the giving
 * callid. If the UI is displaying this call or it's
 * extended one, the topmost panel will be redraw again
 *
 * @param msg Last readed message in Online mode
 */
void
ui_new_msg_refresh(sip_msg_t *msg);

/**
 * @brief Draw keybinding info at the bottom of the panel
 *
 * This function will draw a line with the available keybindings
 * in the last line of the given panel
 *
 */
void
draw_keybindings(PANEL *panel, const char *keybindings[], int count);

/**
 * @brief Draw a vertical scroll
 *
 * This function will draw a vertical scroll in the
 * right or left side of the given window
 */
void
draw_vscrollbar(WINDOW *win, int value, int max, bool left);

/**
 * @brief Draw a message payload in a window
 *
 * Generic drawing function for payload. This function will start
 * writting at 0,0 and return the number of lines written.
 *
 * @param win Ncurses window to draw payload
 * @param msg Msg to be drawn
 */
int
draw_message(WINDOW *win, sip_msg_t *msg);

/**
 * @brief Draw a message payload in a window starting at a given line
 *
 * Generic drawing function for payload. This function will start
 * writting at line starting and first column and return the number
 * of lines written.
 *
 * @param win Ncurses window to draw payload
 * @param msg Msg to be drawn
 * @param starting Number of win line to start writting payload
 */
int
draw_message_pos(WINDOW *win, sip_msg_t *msg, int starting);

#endif    // __SNGREP_UI_MANAGER_H
