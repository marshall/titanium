HKCR
{
  PRODUCT_SHORT_NAME_UQ.ToolsMenuItem.1 = s 'PRODUCT_FRIENDLY_NAME_UQ ToolsMenuItem'
  {
    CLSID = s '{0B4350D1-055F-47A3-B112-5F2F2B0D6F08}'
  }
  PRODUCT_SHORT_NAME_UQ.ToolsMenuItem = s 'PRODUCT_FRIENDLY_NAME_UQ ToolsMenuItem'
  {
    CLSID = s '{0B4350D1-055F-47A3-B112-5F2F2B0D6F08}'
    CurVer = s 'PRODUCT_SHORT_NAME_UQ.ToolsMenuItem.1'
  }
  NoRemove CLSID
  {
    ForceRemove {0B4350D1-055F-47A3-B112-5F2F2B0D6F08} = s 'PRODUCT_FRIENDLY_NAME_UQ ToolsMenuItem'
    {
      ProgID = s 'PRODUCT_SHORT_NAME_UQ.ToolsMenuItem.1'
      VersionIndependentProgID = s 'PRODUCT_SHORT_NAME_UQ.ToolsMenuItem'
      ForceRemove 'Programmable'
      InprocServer32 = s '%MODULE%'
      {
m4_changequote(`^',`^')m4_dnl
m4_ifelse(PRODUCT_OS,^win32^,^m4_dnl
        val ThreadingModel = s 'Apartment'
^,PRODUCT_OS,^wince^,^m4_dnl
        val ThreadingModel = s 'Free'
^)
      }
      val AppID = s '%APPID%'
      'TypeLib' = s '{7708913A-B86C-4D91-B325-657DD5363433}'
    }
  }
}

HKLM
{
  NoRemove SOFTWARE
  {
    NoRemove Microsoft
    {
m4_ifelse(PRODUCT_OS,^win32^,^m4_dnl
      NoRemove 'Internet Explorer'
      {
        NoRemove Extensions
        {
          {09C04DA7-5B76-4EBC-BBEE-B25EAC5965F5}
          {
            val CLSID = s '{1FBA04EE-3024-11d2-8F1F-0000F87ABD16}'
            val ClsidExtension = s '{0B4350D1-055F-47A3-B112-5F2F2B0D6F08}'
          }
        }
      }
^,PRODUCT_OS,^wince^,^m4_dnl
      NoRemove Shell
      {
        NoRemove Extensions
        {
          NoRemove ContextMenus
          {
            NoRemove 'Pocket Internet Explorer'
            {
              NoRemove Tools
              {
                {0B4350D1-055F-47A3-B112-5F2F2B0D6F08} = s 'PRODUCT_FRIENDLY_NAME_UQ Settings'
                {
                }
              }
            }
          }
        }
      }^)
    }
  }
}
