# DOOMERS - Distribution Instructions

## For End Users (Running the Game)

### If you get "DLL not found" errors:

1. **Install Visual C++ Redistributable (REQUIRED)**
   - Download from: https://aka.ms/vs/17/release/vc_redist.x86.exe
   - Run the installer and follow prompts
   - Restart your computer

2. **Make sure all files are together**
   The following files MUST be in the same folder as Doomers.exe:
   - assimp-vc140-mt.dll (or assimp-vc143-mt.dll)
   - glut32.dll
   - glew32.dll
   - msvcp140.dll (if included)
   - vcruntime140.dll (if included)
   - The `res` folder with all game assets

### Common DLL Errors and Solutions:

| Error | Solution |
|-------|----------|
| `msvcp140.dll not found` | Install VC++ Redistributable |
| `vcruntime140.dll not found` | Install VC++ Redistributable |
| `ucrtbase.dll not found` | Install VC++ Redistributable |
| `assimp-vc140-mt.dll not found` | Copy from project folder |
| `glut32.dll not found` | Copy from project Debug folder |

## For Developers (Building from Source)

### Prerequisites:
1. Visual Studio 2022 with C++ Desktop Development workload
2. Windows SDK 10.0 or later

### Build Command:
```cmd
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" OpenGL3DTemplate.sln /p:Configuration=Debug /p:Platform=Win32 /t:Rebuild /m
```

### Creating Distribution Package:
Run `create_distribution.bat` after building to create a folder with all required files.

## Minimum System Requirements:
- Windows 10 or later
- OpenGL 2.0 compatible graphics card
- 512 MB RAM
- 100 MB disk space
