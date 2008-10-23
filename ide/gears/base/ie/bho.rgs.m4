HKCR
{
  PRODUCT_SHORT_NAME_UQ.BHO.1 = s 'PRODUCT_FRIENDLY_NAME_UQ Helper'
  {
    CLSID = s '{E0FEFE40-FBF9-42AE-BA58-794CA7E3FB53}'
  }
  PRODUCT_SHORT_NAME_UQ.BHO = s 'PRODUCT_FRIENDLY_NAME_UQ Helper'
  {
    CLSID = s '{E0FEFE40-FBF9-42AE-BA58-794CA7E3FB53}'
    CurVer = s 'PRODUCT_SHORT_NAME_UQ.BHO.1'
  }
  NoRemove CLSID
  {
    ForceRemove {E0FEFE40-FBF9-42AE-BA58-794CA7E3FB53} = s 'PRODUCT_FRIENDLY_NAME_UQ Helper'
    {
      ProgID = s 'PRODUCT_SHORT_NAME_UQ.BHO.1'
      VersionIndependentProgID = s 'PRODUCT_SHORT_NAME_UQ.BHO'
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
      NoRemove Windows
      {
        NoRemove CurrentVersion
        {
          NoRemove Explorer
          {
            NoRemove 'Browser Helper Objects'
            {
              {E0FEFE40-FBF9-42AE-BA58-794CA7E3FB53} = s 'PRODUCT_FRIENDLY_NAME_UQ Helper'
              {
                val NoExplorer = d 1
              }
            }
          }
        }
      }
    }
  }
}
