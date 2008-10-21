
#ifndef GEARS_DESKTOP_DIR_DIALOG_OSX_H__
#define GEARS_DESKTOP_DIR_DIALOG_OSX_H__

#include <Carbon/Carbon.h>

#include "gears/desktop/file_dialog_osx.h"
#include "gears/desktop/dir_dialog.h"

class DirDialogCarbon : public DirDialog, public FileDialogCarbon {
public:

	DirDialogCarbon();
	virtual ~DirDialogCarbon();

	bool InitDialog(NativeWindowPtr parent,
		const Options& options, std::string16* error);
};

#endif
