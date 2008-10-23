HKCR
{
  NoRemove AppID
  {
    '%APPID%' = s 'PRODUCT_SHORT_NAME_UQ'
    'PRODUCT_SHORT_NAME_UQ.dll'
    {
      val AppID = s '%APPID%'
    }
  }
}
