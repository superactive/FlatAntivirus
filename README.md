# Overview #

A simple malware scanner for Windows that I have done a long time ago for my college assignment. It was created with C/C++ language. The GUI part was created with MFC.

# Screenshots #

Splash screen with background memory scanning

![flatav_splash2.png](http://i62.tinypic.com/30njd5v.jpg)

Main Window

![flatav_maindlg_win7.png](http://i62.tinypic.com/25817o2.jpg)

# Features #

* A malware scanner with graphical user interface
* Scan files and folders
* Scan archive files (only ZIP is supported)
* Scan memory process and start-up items on application start
* If malware is caught, options are given for user to delete or move the malware
* Separate tool to build malware signature database (whitelist is supported)


# Missing features #

* No real-time protection
* No automatic update feature
* Limited archive type scanning


# Build #

### Requirements ###

* Windows XP or higher
* Visual Studio 2008 Professional Edition or higher (MFC library is required). You can also use Visual Studio 2013 Community Edition as it has full support of MFC.
* [ShellFolderTree](https://github.com/sdiwu/ShellFolderTree) component.
* Inno Setup Compiler (optional, for creating installer).

### How to Build ###

* Open the **FlatAntivirus.sln**.
* There are 3 projects inside:
    * FlatAVGUI: main GUI application. It depends on the libflatav project.
    * FVDTool: tool for processing signature database. It depends on the libflatav project.
    * libflatav: AV core engine in DLL form.
* Set FlatAVGUI as start-up project.
* Select the configuration: Debug / DebugU / Release / ReleaseU (the one with suffix "U" is unicode build). Note: MFC for MBCS is not supported anymore in VS 2013.
* Build the solution.
* If libflatav build is success, **libflatav.dll** file will be created in output folder Debug/DebugU/Release/ReleaseU (depends on the configuration) and copied to respective output folder of FVDTool and FlatAVGUI, and FlatAV_Setup folder.
* If FlatAVGUI build is success, **FlatAVGUI.exe** will be created in output folder and copied to FlatAV_Setup folder.


# Run #

* Copy **main.fvd** from FlatAVGUI folder to the output folder (it needs to be in same folder with FlatAVGUI.exe, unless you run the FlatAVGUI directly from Visual Studio). If main.fvd doesn't exist,  you have to build it with FVDTool (read **Create Signature Database** section).
* Build and register **ShellCtl.dll** from **ShellFolderTree** project. You can put the DLL in any folder as long as it has been registered properly.
* Run the FlatAVGUI.exe.


# Create Signature Database #

* The AV signature database file is in main.fvd file.
* Read document inside the fvd_samples folder to understand the FVD format.
* The FVD file is assembled from .cdb, .xdb, .sdb, .wdb, .hdb files using FvdTool.
* Copy FvdTool.exe and libflatav.dll into fvd_samples folder.
* Open command prompt on fvd_samples folder and run this command: FvdTool -b
* It will build and create main.fvd.

Note: some commercial AV will suspect the FvdTool.exe as malware. You can safely ignore it.


# Create Installer #

* Inside FlatAV_Setup you will find Inno Setup Script file **FlatAVSetup.iss** and FlatAV subfolder that contains the required files.
* If FlatAVGUI build is succes, all required binary files will be copied into FlatAV folder, except two files: 
    * **ShellCtl.dll** from ShellControlsEx project
    * **VC++ redist package (vcredist_x86.exe)**. You can download it from [microsoft site](http://support.microsoft.com/kb/2019667). Make sure you download one that match with VS version.
* Open FlatAVSetup.iss with Inno Setup compiler and modify VC++ redist package part accordingly.
* Compile FlatAVSetup.iss.
* If success, **FlatAVsetup.exe** will be created in current folder.

# Contact #

![untitled-2.png](http://i57.tinypic.com/30bfi1e.jpg)
