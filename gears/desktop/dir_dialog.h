#ifndef GEARS_DESKTOP_DIR_DIALOG_H__
#define GEARS_DESKTOP_DIR_DIALOG_H__

#include "gears/desktop/file_dialog.h"

class DirDialog : public FileDialog {
public:
	static DirDialog* Create(const ModuleImplBaseClass* module);	
};

#endif

