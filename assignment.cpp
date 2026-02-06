#include <wx/wx.h>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

enum FileType { File, Dir};

class FileInfo
{
public:
    FileInfo(std::string Name, FileType Type, uintmax_t size, std::time_t Date){
        Name = this.Name;
        Date = this.Date;
        Type = this.Type;
        size = this.size;
    }

private:
    std::string Name;
    std::time_t Date;
    FileType Type;
    uintmax_t size; //std::filesystem::file_size() returns uintmax_t
};


fs::directory_iterator it(fs::current_path());
fs::directory_iterator end_it;
std::vector<FileInfo> files; //dynamic array
for(; it!=end_it; it++) {   //loop thru current folder, each entry puts into an array
    const fs::directory_entry& entry = *it; //dereference iterator, entry is now a file/folder
    uintmax_t size = 0;
    FileType type = fs::is_directory(entry)? Dir : File;
    std::string name = entry.path().filename().string();
    auto ftime = fs::last_write_time(entry);
    std::time_t date = decltype(ftime)::clock::to_time_t(ftime);
    if(type == File) {
        size = fs::file_size(entry);
    }
    FileInfo f(name, type, size, date);
    files.push_back(f);
}

class MyApp :  public wxApp
{
public:
    bool OnInit() override; //override wxApp::OnInit() function, a must
    //itll be called by wxWidgets to initialize the application.
};

class MyFrame : public wxFrame
{
public:
    MyFrame();

private:
    void OnOpen(wxCommandEvent& event); //Open a file using the system's default application for that type of file
    void OnNew(wxCommandEvent& event);  //create a new directory
    void OnRename(wxCommandEvent& event);//prompting the user to enter the new name of the file.
    void OnDelete(wxCommandEvent& event);//Delete a file, prompting the user for confirmation before deletion.
    void OnCopy(wxCommandEvent& event);
    void OnCut(wxCommandEvent& event);
    void OnPaste(wxCommandEvent& event);
    void OnRefresh(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
};

bool MyApp::OnInit()
{
    MyFrame *frame = new MyFrame();
    frame->Show();
    return true;
}

MyFrame::MyFrame()
        : wxFrame(nullptr, wxID_ANY, "FileManager")
{
    wxMenu *menuFile = new wxMenu;
    //Each Append() adds a menu item that users can click
    menuFile->Append(ID_Hello, "&Hello...\tCtrl+H",
                     "Help string shown in status bar for this menu item");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");

    SetMenuBar(menuBar);
    CreateStatusBar();
    SetStatusText("Welcome to wxWidgets!");

    //connect event handlers to the events we want to handle in them.
    Bind(wxEVT_MENU, &MyFrame::OnHello, this, ID_Hello);
    Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
}

void MyFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("This is a wxWidgets Hello World example",
                "About Hello world", wxOK | wxICON_INFORMATION);
}