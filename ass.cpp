#include <wx/wx.h>
#include <wx/listctrl.h>
#include <filesystem>
#include <vector>
#include <string>
#include <ctime>
#include <fstream>

namespace fs = std::filesystem;

// ---------------- FileInfo ----------------
enum FileType { File, Dir };

class FileInfo {
public:
    FileInfo(std::string Name, FileType Type, uintmax_t Size, std::time_t Date)
        : Name(Name), Type(Type), size(Size), Date(Date) {}
    std::string Name;
    FileType Type;
    uintmax_t size;
    std::time_t Date;
};

// ---------------- MyApp ----------------
class MyApp : public wxApp {
public:
    bool OnInit() override;
};

// ---------------- MyFrame ----------------
class MyFrame : public wxFrame {
public:
    MyFrame();
    void LoadDirectory();

private:
    fs::path m_currentDir;
    wxTextCtrl* m_pathText;
    wxListCtrl* m_listCtrl;

    // Virtual clipboard
    fs::path m_clipboardPath;
    bool m_cutOperation = false;

    // Event handlers
    void OnOpen(wxCommandEvent& event);
    void OnNew(wxCommandEvent& event);
    void OnRename(wxCommandEvent& event);
    void OnDelete(wxCommandEvent& event);
    void OnCopy(wxCommandEvent& event);
    void OnCut(wxCommandEvent& event);
    void OnPaste(wxCommandEvent& event);
    void OnRefresh(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnPathEnter(wxCommandEvent& event);
    void OnItemActivated(wxListEvent& event);
};

// ---------------- Event IDs ----------------
enum {
    ID_Open = 1,
    ID_New,
    ID_Rename,
    ID_Delete,
    ID_Copy,
    ID_Cut,
    ID_Paste,
    ID_Refresh
};

// ---------------- MyApp Implementation ----------------
bool MyApp::OnInit() {
    MyFrame* frame = new MyFrame();
    frame->Show();
    return true;
}

// ---------------- MyFrame Implementation ----------------
MyFrame::MyFrame()
    : wxFrame(nullptr, wxID_ANY, "FileManager", wxDefaultPosition, wxSize(800, 500))
{
    m_currentDir = fs::current_path();

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    // Path textbox
    m_pathText = new wxTextCtrl(this, wxID_ANY, m_currentDir.string(),
                                wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    sizer->Add(m_pathText, 0, wxEXPAND | wxALL, 5);

    // File list control
    m_listCtrl = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                wxLC_REPORT | wxLC_SINGLE_SEL);
    m_listCtrl->InsertColumn(0, "Name", wxLIST_FORMAT_LEFT, 300);
    m_listCtrl->InsertColumn(1, "Type", wxLIST_FORMAT_LEFT, 80);
    m_listCtrl->InsertColumn(2, "Size", wxLIST_FORMAT_RIGHT, 100);
    m_listCtrl->InsertColumn(3, "Modified", wxLIST_FORMAT_LEFT, 180);
    sizer->Add(m_listCtrl, 1, wxEXPAND | wxALL, 5);

    SetSizer(sizer);
    CreateStatusBar();
    SetStatusText("Welcome to FileManager!");

    // Menus
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(ID_New, "&New...\tCtrl+N", "Create a new directory");
    menuFile->AppendSeparator();
    menuFile->Append(ID_Open, "&Open...\tCtrl+O", "Open file/directory");
    menuFile->AppendSeparator();
    menuFile->Append(ID_Rename, "&Rename...\tCtrl+E", "Rename file/directory");
    menuFile->AppendSeparator();
    menuFile->Append(ID_Delete, "&Delete...\tDel", "Delete file/directory");

    wxMenu* menuEdit = new wxMenu;
    menuEdit->Append(ID_Copy, "&Copy...\tCtrl+C", "Copy file/directory");
    menuEdit->AppendSeparator();
    menuEdit->Append(ID_Cut, "&Cut...\tCtrl+X", "Cut file/directory");
    menuEdit->AppendSeparator();
    menuEdit->Append(ID_Paste, "&Paste...\tCtrl+V", "Paste file/directory");

    wxMenu* menuView = new wxMenu;
    menuView->Append(ID_Refresh, "&Refresh\tF5", "Refresh directory listing");
    menuView->AppendSeparator();
    menuView->Append(wxID_EXIT, "E&xit", "Exit FileManager");

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuEdit, "&Edit");
    menuBar->Append(menuView, "&View");
    SetMenuBar(menuBar);

    // Bind events
    Bind(wxEVT_MENU, &MyFrame::OnOpen, this, ID_Open);
    Bind(wxEVT_MENU, &MyFrame::OnNew, this, ID_New);
    Bind(wxEVT_MENU, &MyFrame::OnRename, this, ID_Rename);
    Bind(wxEVT_MENU, &MyFrame::OnDelete, this, ID_Delete);
    Bind(wxEVT_MENU, &MyFrame::OnCopy, this, ID_Copy);
    Bind(wxEVT_MENU, &MyFrame::OnCut, this, ID_Cut);
    Bind(wxEVT_MENU, &MyFrame::OnPaste, this, ID_Paste);
    Bind(wxEVT_MENU, &MyFrame::OnRefresh, this, ID_Refresh);
    Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
    m_pathText->Bind(wxEVT_TEXT_ENTER, &MyFrame::OnPathEnter, this);
    m_listCtrl->Bind(wxEVT_LIST_ITEM_ACTIVATED, &MyFrame::OnItemActivated, this);

    LoadDirectory();
}

// ---------------- LoadDirectory ----------------
void MyFrame::LoadDirectory() {
    m_listCtrl->DeleteAllItems();
    int index = 0;

    for (auto& entry : fs::directory_iterator(m_currentDir)) {
        std::string name = entry.path().filename().string();
        std::string typeStr = fs::is_directory(entry) ? "Dir" : "File";
        uintmax_t size = fs::is_directory(entry) ? 0 : fs::file_size(entry);

        auto ftime = fs::last_write_time(entry);
        std::time_t cftime = decltype(ftime)::clock::to_time_t(ftime);
        std::string dateStr = std::asctime(std::localtime(&cftime));
        dateStr.pop_back(); // remove \n

        m_listCtrl->InsertItem(index, name);
        m_listCtrl->SetItem(index, 1, typeStr);
        m_listCtrl->SetItem(index, 2, std::to_string(size));
        m_listCtrl->SetItem(index, 3, dateStr);

        ++index;
    }
}

// ---------------- Event Handlers ----------------
void MyFrame::OnExit(wxCommandEvent&) { Close(true); }

void MyFrame::OnPathEnter(wxCommandEvent&) {
    fs::path newPath(m_pathText->GetValue().ToStdString());
    if (fs::exists(newPath) && fs::is_directory(newPath)) {
        m_currentDir = fs::canonical(newPath);
        m_pathText->SetValue(m_currentDir.string());
        LoadDirectory();
    } else {
        wxMessageBox("Invalid directory", "Error", wxICON_ERROR);
        m_pathText->SetValue(m_currentDir.string());
    }
}

void MyFrame::OnItemActivated(wxListEvent& event) {
    long sel = event.GetIndex();
    std::string name = m_listCtrl->GetItemText(sel);
    fs::path selPath = m_currentDir / name;

    if (fs::is_directory(selPath)) {
        m_currentDir = selPath;
        m_pathText->SetValue(m_currentDir.string());
        LoadDirectory();
    } else {
        wxLaunchDefaultApplication(selPath.string());
    }
}

// ------------- File Operations -------------
void MyFrame::OnOpen(wxCommandEvent&) {
    long sel = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (sel == -1) return;
    fs::path path = m_currentDir / m_listCtrl->GetItemText(sel);
    if (fs::is_directory(path)) {
        m_currentDir = path;
        m_pathText->SetValue(m_currentDir.string());
        LoadDirectory();
    } else {
        wxLaunchDefaultApplication(path.string());
    }
}

void MyFrame::OnNew(wxCommandEvent&) {
    wxTextEntryDialog dlg(this, "Enter directory name:", "New Directory");
    if (dlg.ShowModal() == wxID_OK) {
        fs::path newDir = m_currentDir / dlg.GetValue().ToStdString();
        if (!fs::exists(newDir)) {
            fs::create_directory(newDir);
            SetStatusText("Directory created: " + newDir.string());
            LoadDirectory();
        } else {
            wxMessageBox("Directory already exists!", "Error", wxICON_ERROR);
        }
    }
}

void MyFrame::OnRename(wxCommandEvent&) {
    long sel = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (sel == -1) return;
    std::string oldName = m_listCtrl->GetItemText(sel);
    fs::path oldPath = m_currentDir / oldName;

    wxTextEntryDialog dlg(this, "Enter new name:", "Rename", oldName);
    if (dlg.ShowModal() == wxID_OK) {
        fs::path newPath = m_currentDir / dlg.GetValue().ToStdString();
        if (!fs::exists(newPath)) {
            fs::rename(oldPath, newPath);
            SetStatusText("Renamed: " + oldName + " -> " + newPath.string());
            LoadDirectory();
        } else {
            wxMessageBox("Target name already exists!", "Error", wxICON_ERROR);
        }
    }
}

void MyFrame::OnDelete(wxCommandEvent&) {
    long sel = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (sel == -1) return;
    std::string name = m_listCtrl->GetItemText(sel);
    fs::path path = m_currentDir / name;

    if (wxMessageBox("Delete " + name + "?", "Confirm", wxYES_NO | wxICON_WARNING) == wxYES) {
        if (fs::is_directory(path)) fs::remove_all(path);
        else fs::remove(path);
        SetStatusText("Deleted: " + name);
        LoadDirectory();
    }
}

void MyFrame::OnCopy(wxCommandEvent&) {
    long sel = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (sel == -1) return;
    m_clipboardPath = m_currentDir / m_listCtrl->GetItemText(sel);
    m_cutOperation = false;
    SetStatusText("Copied: " + m_clipboardPath.string());
}

void MyFrame::OnCut(wxCommandEvent&) {
    long sel = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (sel == -1) return;
    m_clipboardPath = m_currentDir / m_listCtrl->GetItemText(sel);
    m_cutOperation = true;
    SetStatusText("Cut: " + m_clipboardPath.string());
}

void MyFrame::OnPaste(wxCommandEvent&) {
    if (m_clipboardPath.empty()) return;
    fs::path dest = m_currentDir / m_clipboardPath.filename();

    if (fs::exists(dest)) {
        if (wxMessageBox("Overwrite existing file/directory?", "Confirm", wxYES_NO | wxICON_QUESTION) != wxYES)
            return;
    }

    try {
        if (m_cutOperation) {
            fs::rename(m_clipboardPath, dest);
            SetStatusText("Moved: " + dest.string());
        } else {
            if (fs::is_directory(m_clipboardPath))
                fs::copy(m_clipboardPath, dest, fs::copy_options::recursive);
            else
                fs::copy_file(m_clipboardPath, dest, fs::copy_options::overwrite_existing);
            SetStatusText("Copied: " + dest.string());
        }
        m_clipboardPath.clear();
        m_cutOperation = false;
        LoadDirectory();
    } catch (...) {
        wxMessageBox("Failed to paste file/directory.", "Error", wxICON_ERROR);
    }
}

void MyFrame::OnRefresh(wxCommandEvent&) { LoadDirectory(); }

// ---------------- Main ----------------
wxIMPLEMENT_APP(MyApp);