HKCR
{
  PRODUCT_SHORT_NAME_UQ.PIEDialogBridge.1 = s 'PRODUCT_FRIENDLY_NAME_UQ PIEDialogBridge'
  {
    CLSID = s '{134AB400-1A81-4fc8-85DD-29CD51E9D6DE}'
  }
  PRODUCT_SHORT_NAME_UQ.PIEDialogBridge = s 'PRODUCT_FRIENDLY_NAME_UQ PIEDialogBridge'
  {
    CLSID = s '{134AB400-1A81-4fc8-85DD-29CD51E9D6DE}'
    CurVer = s 'PRODUCT_SHORT_NAME_UQ.PIEDialogBridge.1'
  }
  NoRemove CLSID
  {
    ForceRemove {134AB400-1A81-4fc8-85DD-29CD51E9D6DE} = s 'PRODUCT_FRIENDLY_NAME_UQ PIEDialogBridge'
    {
      ProgID = s 'PRODUCT_SHORT_NAME_UQ.PIEDialogBridge.1'
      VersionIndependentProgID = s 'PRODUCT_SHORT_NAME_UQ.PIEDialogBridge'
      ForceRemove 'Programmable'
      InprocServer32 = s '%MODULE%'
      {
        val ThreadingModel = s 'Free'
      }
      val AppID = s '%APPID%'
      'TypeLib' = s '{7708913A-B86C-4D91-B325-657DD5363433}'
    }
  }
}
