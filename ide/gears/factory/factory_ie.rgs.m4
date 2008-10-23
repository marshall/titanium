HKCR
{
  PRODUCT_SHORT_NAME_UQ.Factory.1 = s 'PRODUCT_FRIENDLY_NAME_UQ Factory'
  {
    CLSID = s '{C93A7319-17B3-4504-87CD-03EFC6103E6E}'
  }
  PRODUCT_SHORT_NAME_UQ.Factory = s 'PRODUCT_FRIENDLY_NAME_UQ Factory'
  {
    CLSID = s '{C93A7319-17B3-4504-87CD-03EFC6103E6E}'
    CurVer = s 'PRODUCT_SHORT_NAME_UQ.Factory.1'
  }
  NoRemove CLSID
  {
    ForceRemove {C93A7319-17B3-4504-87CD-03EFC6103E6E} = s 'PRODUCT_FRIENDLY_NAME_UQ Factory'
    {
      ProgID = s 'PRODUCT_SHORT_NAME_UQ.Factory.1'
      VersionIndependentProgID = s 'PRODUCT_SHORT_NAME_UQ.Factory'
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
