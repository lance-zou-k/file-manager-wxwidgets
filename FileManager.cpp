#include "FileManager.h"
#include <wx/msgdlg.h>
#include <wx/textdlg.h>
#include <wx/listctrl.h>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <filesystem>
namespace fs = std::filesystem;

// The Event Table maps UI clicks to the functions below
wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(wxID_OPEN, MyFrame::OnOpen)
    EVT_MENU(ID_NEWDIR, MyFrame::OnNewDir)
    EVT_MENU(ID_RENAME, MyFrame::OnRename)
    EVT_MENU(wxID_DELETE, MyFrame::OnDelete)
    EVT_MENU(wxID_COPY, MyFrame::OnCopy)
    EVT_MENU(wxID_CUT, MyFrame::OnCut)
    EVT_MENU(wxID_PASTE, MyFrame::OnPaste)
    EVT_MENU(ID_REFRESH, MyFrame::OnRefresh)
    EVT_MENU(wxID_EXIT, MyFrame::OnExit)
    EVT_TEXT_ENTER(wxID_ANY, MyFrame::OnPathEnter)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, MyFrame::OnFileActivated)
wxEND_EVENT_TABLE()

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit() {
    MyFrame* frame = new MyFrame("C++ wxWidgets File Manager");
    frame->Show(true);
    return true;
}

MyFrame::MyFrame(const wxString& title) 
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600)) {
    
    // 1. Setup Menu Bar
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(wxID_OPEN, "&Open\tCtrl+O");
    menuFile->Append(ID_NEWDIR, "&New Directory\tCtrl+N");
    menuFile->Append(ID_RENAME, "&Rename\tCtrl+R");
    menuFile->Append(wxID_DELETE, "&Delete\tDel");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_COPY, "&Copy\tCtrl+C");
    menuFile->Append(wxID_CUT, "Cu&t\tCtrl+X");
    menuFile->Append(wxID_PASTE, "&Paste\tCtrl+V");
    menuFile->AppendSeparator();
    menuFile->Append(ID_REFRESH, "Re&fresh\tF5");
    menuFile->Append(wxID_EXIT, "E&xit");

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    SetMenuBar(menuBar);

    // 2. Setup Layout
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    
    m_pathBar = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    sizer->Add(m_pathBar, 0, wxEXPAND | wxALL, 5);

    m_fileList = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
    m_fileList->AppendColumn("Name", wxLIST_FORMAT_LEFT, 250);
    m_fileList->AppendColumn("Type", wxLIST_FORMAT_LEFT, 80);
    m_fileList->AppendColumn("Size", wxLIST_FORMAT_RIGHT, 100);
    m_fileList->AppendColumn("Date Modified", wxLIST_FORMAT_LEFT, 200);
    sizer->Add(m_fileList, 1, wxEXPAND | wxALL, 5);

    m_statusBar = CreateStatusBar();
    SetSizer(sizer);

    UpdateDisplay(fs::current_path());
}

void MyFrame::UpdateDisplay(fs::path newPath) {
    if (fs::exists(newPath) && fs::is_directory(newPath)) {
        m_currentPath = newPath;
        m_pathBar->SetValue(m_currentPath.string());
        RefreshList();
    }
}

void MyFrame::RefreshList() {
    m_fileList->DeleteAllItems();
    long index = 0;
    try {
        for (const auto& entry : fs::directory_iterator(m_currentPath)) {
            // Column 0: Name
            m_fileList->InsertItem(index, entry.path().filename().string());
            
            // Column 1: Type
            if (fs::is_directory(entry)) {
                m_fileList->SetItem(index, 1, "Folder");
                m_fileList->SetItem(index, 2, "--");
            } else {
                m_fileList->SetItem(index, 1, "File");
                // Column 2: Size
                m_fileList->SetItem(index, 2, std::to_string(fs::file_size(entry) / 1024) + " KB");
            }

            // Column 3: Date Modified
            auto ftime = fs::last_write_time(entry);
            
            // Convert file_time to system_time
            auto s_time = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
            );
            std::time_t tt = std::chrono::system_clock::to_time_t(s_time);
            
            // Format time as YYYY-MM-DD HH:MM
            char buffer[20];
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", std::localtime(&tt));
            
            m_fileList->SetItem(index, 3, buffer);

            index++;
        }
    } catch (const fs::filesystem_error& e) {
        m_statusBar->SetStatusText("Access Denied: " + wxString(e.what()));
    }
}

fs::path MyFrame::GetSelectedPath() {
    long item = m_fileList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (item == -1) return "";
    return m_currentPath / m_fileList->GetItemText(item).ToStdString();
}

// --- Event Handlers ---

void MyFrame::OnOpen(wxCommandEvent& event) {
    fs::path target = GetSelectedPath();
    if (!target.empty()) {
        DoOpenFile(target);
    }
}

void MyFrame::OnFileActivated(wxListEvent& event) {
    wxString fileName = event.GetLabel();
    fs::path target = m_currentPath / fileName.ToStdString();
    
    DoOpenFile(target);
}

void MyFrame::DoOpenFile(const fs::path& path) {
    if (!fs::exists(path)) return;

    if (fs::is_directory(path)) {
        UpdateDisplay(path);
    } else {
        m_statusBar->SetStatusText("wxWidgets is choosing an app for: " + path.filename().string());
        bool success = wxLaunchDefaultApplication(path.string());

        if (!success) {
            wxMessageBox("wxWidgets could not find a default app for this file type.\n"
                         "Check if a Linux app (like gedit) is associated with it.", 
                         "Association Error", wxOK | wxICON_ERROR);
        }
    }
}

void MyFrame::OnNewDir(wxCommandEvent& event) {
    wxTextEntryDialog dlg(this, "Enter directory name:", "New Folder");
    if (dlg.ShowModal() == wxID_OK) {
        fs::create_directory(m_currentPath / dlg.GetValue().ToStdString());
        RefreshList();
    }
}

void MyFrame::OnRename(wxCommandEvent& event) {
    fs::path oldPath = GetSelectedPath();
    if (oldPath.empty()) return;

    wxTextEntryDialog dlg(this, "New name:", "Rename", oldPath.filename().string());
    if (dlg.ShowModal() == wxID_OK) {
        fs::rename(oldPath, oldPath.parent_path() / dlg.GetValue().ToStdString());
        RefreshList();
    }
}

void MyFrame::OnDelete(wxCommandEvent& event) {
    fs::path target = GetSelectedPath();
    if (target.empty()) return;

    if (wxMessageBox("Delete?", "Confirm", wxYES_NO) == wxYES) {
        fs::remove_all(target);
        RefreshList();
    }
}

void MyFrame::OnCopy(wxCommandEvent& event) {
    m_clipboardPath = GetSelectedPath();
    m_isCutOp = false;
    m_statusBar->SetStatusText("Copied to clipboard");
}

void MyFrame::OnCut(wxCommandEvent& event) {
    m_clipboardPath = GetSelectedPath();
    m_isCutOp = true;
    m_statusBar->SetStatusText("Cut to clipboard");
}

void MyFrame::OnPaste(wxCommandEvent& event) {
    if (m_clipboardPath.empty()) return;
    fs::path dest = m_currentPath / m_clipboardPath.filename();
    
    if (m_isCutOp) fs::rename(m_clipboardPath, dest);
    else fs::copy(m_clipboardPath, dest, fs::copy_options::recursive);
    
    m_clipboardPath.clear();
    RefreshList();
    m_statusBar->SetStatusText("Paste complete");
}

void MyFrame::OnPathEnter(wxCommandEvent& event) {
    UpdateDisplay(fs::path(m_pathBar->GetValue().ToStdString()));
}

void MyFrame::OnRefresh(wxCommandEvent& event) { RefreshList(); }
void MyFrame::OnExit(wxCommandEvent& event) { Close(true); }