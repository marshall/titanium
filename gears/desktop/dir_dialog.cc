#include "gears/desktop/dir_dialog.h"

#ifdef OS_MACOSX
#include "gears/desktop/dir_dialog_osx.h"
#endif

DirDialog* DirDialog::Create(const ModuleImplBaseClass* module)
{
  NativeWindowPtr parent = NULL;
  GetBrowserWindow(module, &parent);
#if defined(OS_MACOSX)
  DirDialog* dialog = new DirDialogCarbon;
#else
  DirDialog* dialog = NULL;
#endif
  if (dialog)
    dialog->Init(module, parent);
  return dialog;
}
