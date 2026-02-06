#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;


enum {
    ID_RENAME = 1001,
    ID_NEWDIR,
    ID_REFRESH
};

class MyApp : public wxApp {
public:
    virtual bool OnInit();
};

class MyFrame : public wxFrame {
public:
    MyFrame(const wxString& title);

private:
    // UI Elements
    wxTextCtrl* m_pathBar;
    wxListCtrl* m_fileList;
    wxStatusBar* m_statusBar;

    // Internal State
    fs::path m_currentPath;
    fs::path m_clipboardPath;
    bool m_isCutOp = false;

    // Helper Methods
    void DoOpenFile(const fs::path& path);
    void UpdateDisplay(fs::path newPath);
    void RefreshList();
    fs::path GetSelectedPath();

    // Event Handlers
    void OnOpen(wxCommandEvent& event);
    void OnFileActivated(wxListEvent& event);
    void OnNewDir(wxCommandEvent& event);
    void OnRename(wxCommandEvent& event);
    void OnDelete(wxCommandEvent& event);
    void OnCopy(wxCommandEvent& event);
    void OnCut(wxCommandEvent& event);
    void OnPaste(wxCommandEvent& event);
    void OnRefresh(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnPathEnter(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};

// IDs for Menu Items


#endif