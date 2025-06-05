# YTX File Editor
This is a tool that allows you to easily open and edit Yuke's text files(.ytx). It was made specifically
with UFC Undisputed 3 files in mind, but it may work with other games if their files follow the same format.

## Features
- View all string entries contained in a file.
- Edit any entry without a character limit.
- Add new string entries to a file.

## Usage
1. Clone and build the project.
2. Open `YTX-File-Editor.exe` or execute it with the args `-v 1` to enable console logging.
3. Either select a file with the "Browse" button or paste the path to a `.ytx` file in the text box.
4. Press the "Load File" button.
5. Make the changes you wish to in the file.
6. Press the "Save Changes" button.

Saving the changes will overwrite the original file, so it is recommended to backup your files beforehand,
even though a copy of your file is already automatically once you open it under the same name with the 
`.backup` extension.
