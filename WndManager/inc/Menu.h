#pragma once

#include "Wnd.h"
#include "WndShade.h"

#define MENU_TYPE_MASK  0x01f0
#define MENU_END        0x0000
#define MENU_ITEM       0x0100
#define MENU_SEPARATOR  0x0110

#define MENU_STATE_MASK 0x001f
#define MENU_NOTINITED       0
#define MENU_NORMAL          1
#define MENU_DISABLED        2
#define MENU_SELECTED        4
#define MENU_CHECKED         8

//
struct menu
{
    int         type;
    std::string name;
    input_t     code;
    std::string helpLine;

    input_t     iKey;
    pos_t       x;
    pos_t       y;
    pos_t       size;
};

using menu_list = std::vector<menu>;

class Shade;

class Menu : public Wnd
{
protected:
    menu_list               m_menu;
    std::vector<cell_t>     m_saveBlock;
    std::unique_ptr<Shade>  m_shade;
    std::unique_ptr<Menu>   m_nextMenu;

    bool                    m_fMain{ false };
    int                     m_selected{-1};
    bool                    m_leftMKey{ false };

public:
    Menu() = delete;
    explicit Menu(const menu_list& menu, pos_t x, pos_t y) : m_menu{ menu } {
        m_left = x; m_top = y;
    }
    virtual ~Menu() { Close(0); }

    bool    IsActive() {return !m_menu.empty();}
    int     GetNextItem(int n);
    int     GetPrevItem(int n);

    virtual input_t Close(input_t code);
    virtual input_t Activate(bool capture = false) = 0;

    //from Wnd class
    virtual wnd_t   GetWndType() const override {return wnd_t::menu;}
    virtual bool    Refresh() override = 0;
};

class LineMenu : public Menu
{
public:
    LineMenu() = delete;
    explicit LineMenu(const menu_list& menu, pos_t x = 0, pos_t y = 0) : Menu(menu, x, y) {}
    virtual ~LineMenu() = default;

    virtual input_t Close(input_t code) override final;
    virtual input_t Activate(bool fCapture = false) override final;
    virtual bool    Refresh() override final;
    virtual input_t EventProc(input_t code) override final;
};

class FrameMenu : public Menu
{
    pos_t     m_realSizey{};  //real y size

public:
    FrameMenu() = delete;
    explicit FrameMenu(const menu_list& menu, pos_t x, pos_t y) : Menu(menu, x, y) {}
    virtual ~FrameMenu() = default;

    virtual input_t Close(input_t code) override {return Menu::Close(code);}
    virtual input_t Activate(bool fCapture = false) override;
    virtual bool    Refresh() override;
    virtual input_t EventProc(input_t code) override;
};

class PopupMenu final : public FrameMenu
{
public:
    PopupMenu() = delete;
    explicit PopupMenu(const menu_list& menu, pos_t x, pos_t y) : FrameMenu(menu, x, y) {}
    virtual ~PopupMenu() = default;

    virtual input_t Close(input_t code) override final      {return FrameMenu::Close(code);}
    virtual input_t Activate(bool fCapture = true) override final;
    virtual bool    Refresh() override final                {return FrameMenu::Refresh();}
    virtual input_t EventProc(input_t code) override final  {return FrameMenu::EventProc(code);}
};
