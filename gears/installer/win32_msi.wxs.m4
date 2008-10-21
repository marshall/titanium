m4_changequote(`[',`]')m4_dnl
m4_changequote([~`],[`~])m4_dnl
m4_define(~`m4_underscore`~, ~`m4_translit(~`$1`~,~`-`~,~`_`~)`~)m4_dnl
m4_divert(~`-1`~)
# foreach(x, (item_1, item_2, ..., item_n), stmt)
#   parenthesized list, simple version
m4_define(~`m4_foreach`~,
~`m4_pushdef(~`$1`~)_foreach($@)m4_popdef(~`$1`~)`~)
m4_define(~`_arg1`~, ~`$1`~)
m4_define(~`_foreach`~, ~`m4_ifelse(~`$2`~, ~`()`~, ~``~,
  ~`m4_define(~`$1`~, _arg1$2)$3~``~$0(~`$1`~, (m4_shift$2), ~`$3`~)`~)`~)
m4_divert~``~m4_dnl
<?xml version='1.0' ?>

<!--
Copyright 2007, Google Inc.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, 
    this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
 3. Neither the name of Google Inc. nor the names of its contributors may be
    used to endorse or promote products derived from this software without
    specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-->

<Wix xmlns='http://schemas.microsoft.com/wix/2006/wi'>
  <Product Id='$(var.OurWin32ProductId)' Name='PRODUCT_FRIENDLY_NAME_UQ'
    Language='1033' Version='$(var.OurMsiVersion)'
    Manufacturer='Google' UpgradeCode='D91DF85A-1C3B-4d62-914B-DEEEF73AD78C'>
    <Package Description='PRODUCT_FRIENDLY_NAME_UQ'
      Comments='PRODUCT_FRIENDLY_NAME_UQ' Manufacturer='Google'
      InstallerVersion='200' Compressed='yes' />
    <Media Id='1' Cabinet='product.cab' EmbedCab='yes'
      CompressionLevel='high' />
    <Upgrade Id='D91DF85A-1C3B-4d62-914B-DEEEF73AD78C'>
      <UpgradeVersion Property='UPGRADING' OnlyDetect='no'
        Minimum='0.0.0.0' IncludeMinimum='yes'
        Maximum='$(var.OurMsiVersion)' IncludeMaximum='no' />
      <UpgradeVersion Property='NEWERVERSIONDETECTED' OnlyDetect='yes'
        Minimum='$(var.OurMsiVersion)' IncludeMinimum='no' />
    </Upgrade>
    <Directory Id='TARGETDIR' Name='SourceDir'>
      <Directory Id='ProgramFilesFolder' Name='PFiles'>
        <Directory Id='GoogleDir' Name='Google'>
          <Directory Id='ProductDir' Name='PRODUCT_FRIENDLY_NAME_UQ'>

            <Component Id='OurIERegistry' Guid='$(var.OurComponentGUID_IERegistry)'>
              <RegistryValue
                Root='HKLM' Key='Software\Google\Update\Clients\{283EAF47-8817-4c2b-A801-AD1FADFB7BAA}'
                Name='pv' Value='PRODUCT_VERSION'
                Action='write' Type='string' />
              <RegistryValue
                Root='HKLM' Key='Software\Google\Update\Clients\{283EAF47-8817-4c2b-A801-AD1FADFB7BAA}'
                Name='ap' Value=''
                Action='write' Type='string' />
              <!-- Automatically enable control by adding to IE7's pre-approved list. See
                http://msdn.microsoft.com/library/default.asp?url=/library/en-us/IETechCol/cols/dnexpie/activex_security.asp?frame=true -->
              <RegistryKey Root='HKLM' Action='createAndRemoveOnUninstall'
                Key='Software\Microsoft\Windows\CurrentVersion\Ext\PreApproved\{C93A7319-17B3-4504-87CD-03EFC6103E6E}' />
              <!-- For vista, a setting that allows us to silently load the our dll with low integrity using rundll32.exe. 
                See http://msdn2.microsoft.com/en-us/library/bb250462.aspx -->
              <RegistryKey Root='HKLM' Action='createAndRemoveOnUninstall'
                Key='Software\Microsoft\Internet Explorer\Low Rights\RunDll32Policy\PRODUCT_SHORT_NAME_UQ.dll' />
              <!-- For vista, a setting that allows us to silently run vista_broker.exe with medium integrity level (3) -->
              <RegistryValue
                Root='HKLM' Key='Software\Microsoft\Internet Explorer\Low Rights\ElevationPolicy\{A553FC79-0F5A-4DDE-A7AE-920F6EE4E264}'
                Name='AppName' Value='vista_broker.exe'
                Action='write' Type='string' />
              <RegistryValue
                Root='HKLM' Key='Software\Microsoft\Internet Explorer\Low Rights\ElevationPolicy\{A553FC79-0F5A-4DDE-A7AE-920F6EE4E264}'
                Name='AppPath' Value='[OurIEVersionedDir]'
                Action='write' Type='string' />
              <RegistryValue
                Root='HKLM' Key='Software\Microsoft\Internet Explorer\Low Rights\ElevationPolicy\{A553FC79-0F5A-4DDE-A7AE-920F6EE4E264}'
                Name='Policy' Value='3'
                Action='write' Type='integer' />
            </Component>

            <Component Id='OurFFRegistry' Guid='$(var.OurComponentGUID_FFRegistry)'>
              <!-- IMPORTANT: the 'Name' field below MUST match our <em:id> value in install.rdf.m4 -->
              <RegistryValue
                Root='HKLM' Key='Software\Mozilla\Firefox\Extensions'
                Name='{000a9d1c-beef-4f90-9363-039d445309b8}' Value='[OurFFDir]'
                Action='write' Type='string' />
            </Component>

            <!-- [naming] --> 
            <Component Id='OurSharedRegistry' Guid='$(var.OurComponentGUID_SharedRegistry)'>
              <RegistryValue
                Root='HKLM' Key='Software\Google\Gears'
                Name='install_dir' Value='[ProductDir]'
                Action='write' Type='string' />
              <RegistryValue
                Root='HKLM' Key='Software\Google\Gears'
                Name='install_ver' Value='PRODUCT_VERSION'
                Action='write' Type='string' />
            </Component>

            <!-- IMPORTANT: the OurShared* 'Name' fields MUST match the WIN32 paths in /{firefox,ie}/PathUtils.cc -->
            <Directory Id='OurSharedDir' Name='Shared'>
              <Component Id='OurSharedDirFiles' Guid='$(var.OurComponentGUID_SharedFiles)'>
                <File Id='notifier_exe' Name='notifier.exe' DiskId='1'
                  Source="$(var.OurCommonPath)/notifier.exe" />
              </Component>
              <Directory Id='OurSharedVersionedDir' Name='PRODUCT_VERSION'>
                <Component Id='OurSharedVersionedDirFiles' Guid='$(var.OurComponentGUID_SharedVersionedFiles)'>
                  <File Id='notifier_dll' Name='notifier.dll' DiskId='1'
                    Source="$(var.OurCommonPath)/notifier.dll" />
                  <File Id='shared_crash_sender' Name='crash_sender.exe' DiskId='1'
                    Source="$(var.OurCommonPath)/crash_sender.exe" />
m4_ifelse(~`OFFICIAL_BUILD`~,~`m4dnl
`~,~`m4dnl
                  <File Id='shared_crash_sender_pdb' Name='crash_sender.pdb' DiskId='1'
                    Source="$(var.OurCommonPath)/crash_sender.pdb" />
`~)
                </Component>
              </Directory>
            </Directory>

            <Directory Id='OurIEDir' Name='Internet Explorer'>
              <Directory Id='OurIEVersionedDir' Name='PRODUCT_VERSION'>
                <Component Id='OurIEDirFiles' Guid='$(var.OurComponentGUID_IEFiles)'>
                  <File Id='ie_crash_sender' Name='crash_sender.exe' DiskId='1'
                    Source="$(var.OurCommonPath)/crash_sender.exe" />
m4_ifelse(~`OFFICIAL_BUILD`~,~`m4dnl
`~,~`m4dnl
                  <File Id='ie_crash_sender_pdb' Name='crash_sender.pdb' DiskId='1'
                    Source="$(var.OurCommonPath)/crash_sender.pdb" />
`~)
                  <File Id='ie_dll' Name='PRODUCT_SHORT_NAME_UQ.dll' DiskId='1'
                    Source="$(var.OurIEPath)/PRODUCT_SHORT_NAME_UQ.dll" SelfRegCost='1' />
m4_ifelse(~`OFFICIAL_BUILD`~,~`m4dnl
`~,~`m4dnl
                  <File Id='ie_pdb' Name='PRODUCT_SHORT_NAME_UQ.pdb' DiskId='1'
                    Source="$(var.OurIEPath)/PRODUCT_SHORT_NAME_UQ.pdb" />
`~)
                  <File Id='vista_broker' Name='vista_broker.exe' DiskId='1'
                    Source="$(var.OurIEPath)/vista_broker.exe" />
m4_ifelse(~`OFFICIAL_BUILD`~,~`m4dnl
`~,~`m4dnl
                  <File Id='vista_broker_pdb' Name='vista_broker.pdb' DiskId='1'
                    Source="$(var.OurIEPath)/vista_broker.pdb" />
`~)
m4_ifdef(~`USING_CCTESTS`~,~`m4_dnl
                  <File Id='ie_notifier_test_exe' Name='notifier_test.exe' DiskId='1'
                    Source="$(var.OurCommonPath)/notifier_test.exe" />
                  <File Id='ie_ipc_test_exe' Name='ipc_test.exe' DiskId='1'
                    Source="$(var.OurIpcTestPath)/ipc_test.exe" />
`~)
                </Component>
              </Directory>
            </Directory>

            <!-- Firefox avoids a versioned dir because the Extension Manager
                 sometimes gets confused and disables the updated version. -->
            <Directory Id='OurFFDir' Name='Firefox'>
              <Component Id='OurFFDirFiles' Guid='$(var.OurComponentGUID_FFDirFiles)'>
                <File Id='ff_chrome.manifest' Name='chrome.manifest' DiskId='1'
                  Source="$(var.OurFFPath)/chrome.manifest" />
                <File Id='ff_crash_sender' Name='crash_sender.exe' DiskId='1'
                  Source="$(var.OurCommonPath)/crash_sender.exe" />
m4_ifelse(~`OFFICIAL_BUILD`~,~`m4dnl
`~,~`m4dnl
                <File Id='ff_crash_sender_pdb' Name='crash_sender.pdb' DiskId='1'
                  Source="$(var.OurCommonPath)/crash_sender.pdb" />
`~)
                <File Id='ff_install.rdf' Name='install.rdf' DiskId='1'
                  Source="$(var.OurFFPath)/install.rdf" />
              </Component>
              <Directory Id='OurFFComponentsDir' Name='components'>
                <Component Id='OurFFComponentsDirFiles'
                  Guid='$(var.OurComponentGUID_FFComponentsDirFiles)'>
                  <File Id='ff_bootstrap.js' Name='bootstrap.js'
                    DiskId='1' Source="$(var.OurFFPath)/components/bootstrap.js" />
                  <File Id='ff_xpt' Name='PRODUCT_SHORT_NAME_UQ.xpt' DiskId='1'
                    Source="$(var.OurFFPath)/components/PRODUCT_SHORT_NAME_UQ.xpt" />
                  <File Id='ff2_dll' Name='PRODUCT_SHORT_NAME_UQ~``~_ff2.dll' DiskId='1'
                    Source="$(var.OurFFPath)/components/PRODUCT_SHORT_NAME_UQ~``~_ff2.dll" />
m4_ifelse(~`OFFICIAL_BUILD`~,~`m4dnl
`~,~`m4dnl
                  <File Id='ff2_pdb' Name='PRODUCT_SHORT_NAME_UQ~``~_ff2.pdb' DiskId='1'
                    Source="$(var.OurFFPath)/components/PRODUCT_SHORT_NAME_UQ~``~_ff2.pdb" />
`~)
                  <File Id='ff3_dll' Name='PRODUCT_SHORT_NAME_UQ.dll' DiskId='1'
                    Source="$(var.OurFFPath)/components/PRODUCT_SHORT_NAME_UQ.dll" />
m4_ifelse(~`OFFICIAL_BUILD`~,~`m4dnl
`~,~`m4dnl
                  <File Id='ff3_pdb' Name='PRODUCT_SHORT_NAME_UQ.pdb' DiskId='1'
                    Source="$(var.OurFFPath)/components/PRODUCT_SHORT_NAME_UQ.pdb" />
`~)
m4_ifdef(~`USING_CCTESTS`~,~`m4_dnl
                  <File Id='ff_notifier_test_exe' Name='notifier_test.exe' DiskId='1'
                    Source="$(var.OurCommonPath)/notifier_test.exe" />
                  <File Id='ff_ipc_test_exe' Name='ipc_test.exe' DiskId='1'
                    Source="$(var.OurIpcTestPath)/ipc_test.exe" />
`~)
                </Component>
              </Directory>

              <!-- Begin: resource lists that MUST be kept in sync with 'rules.mk' -->
              <Directory Id='OurFFChromeDir' Name='chrome'>
                <Directory Id='OurFFChromeFilesDir' Name='chromeFiles'>
                  <Directory Id='OurFFContentDir' Name='content'>
                    <Component Id='OurFFContentDirFiles'
                      Guid='$(var.OurComponentGUID_FFContentDirFiles)'>
                      <File Id='button_bg.gif' Name='button_bg.gif'
                        DiskId='1' Source="$(var.OurFFPath)/chrome/chromeFiles/content/button_bg.gif" />
                      <File Id='button_corner_black.gif' Name='button_corner_black.gif'
                        DiskId='1' Source="$(var.OurFFPath)/chrome/chromeFiles/content/button_corner_black.gif" />
                      <File Id='button_corner_blue.gif' Name='button_corner_blue.gif'
                        DiskId='1' Source="$(var.OurFFPath)/chrome/chromeFiles/content/button_corner_blue.gif" />
                      <File Id='button_corner_grey.gif' Name='button_corner_grey.gif'
                        DiskId='1' Source="$(var.OurFFPath)/chrome/chromeFiles/content/button_corner_grey.gif" />
                      <File Id='ff_browser_overlay.js' Name='browser-overlay.js'
                        DiskId='1' Source="$(var.OurFFPath)/chrome/chromeFiles/content/browser-overlay.js" />
                      <File Id='ff_browser_overlay.xul' Name='browser-overlay.xul'
                        DiskId='1' Source="$(var.OurFFPath)/chrome/chromeFiles/content/browser-overlay.xul" />
                      <File Id='ff_icon_32x32.png' Name='icon_32x32.png'
                        DiskId='1' Source="$(var.OurFFPath)/chrome/chromeFiles/content/icon_32x32.png" />
                      <File Id='ff_local_data.png' Name='local_data.png'
                        DiskId='1' Source="$(var.OurFFPath)/chrome/chromeFiles/content/local_data.png" />
                      <File Id='ff_location_data.png' Name='location_data.png'
                        DiskId='1' Source="$(var.OurFFPath)/chrome/chromeFiles/content/location_data.png" />
                      <File Id='ff_permissions_dialog.html' Name='permissions_dialog.html'
                        DiskId='1' Source="$(var.OurFFPath)/chrome/chromeFiles/content/permissions_dialog.html" />
                      <File Id='ff_settings_dialog.html' Name='settings_dialog.html'
                        DiskId='1' Source="$(var.OurFFPath)/chrome/chromeFiles/content/settings_dialog.html" />
                      <File Id='ff_shortcuts_dialog.html' Name='shortcuts_dialog.html'
                        DiskId='1' Source="$(var.OurFFPath)/chrome/chromeFiles/content/shortcuts_dialog.html" />
                    </Component>
                  </Directory>
                  <Directory Id='OurFFLocaleDir' Name='locale'>
m4_foreach(~`LANG`~, I18N_LANGUAGES, ~`m4_dnl
                    <Directory Id='~`Our`~m4_underscore(LANG)~`Dir`~' Name='LANG'>
                      <Component Id='~`OurFF`~m4_underscore(LANG)~`DirFiles`~'
                        Guid='$(var.~`OurComponentGUID_FFLang`~m4_underscore(LANG)~`DirFiles`~)'>
                        <File Id='~`ff_`~m4_underscore(LANG)~`_i18n.dtd`~' Name='i18n.dtd'
                          DiskId='1' Source="$(var.OurFFPath)/chrome/chromeFiles/locale/LANG/i18n.dtd" />
                      </Component>
                    </Directory>
`~)m4_dnl
                  </Directory>
                </Directory>
              </Directory>
              <!-- End: resource lists that MUST be kept in sync with 'rules.mk' -->

              <Directory Id='OurFFLibDir' Name='lib'>
                <Component Id='OurFFLibDirFiles'
                  Guid='$(var.OurComponentGUID_FFLibDirFiles)'>
                  <File Id='ff_updater.js' Name='updater.js'
                    DiskId='1' Source="$(var.OurFFPath)/lib/updater.js" />
                </Component>
              </Directory>
            </Directory>

          </Directory>
        </Directory>
      </Directory>
    </Directory>
    <Feature Id='PRODUCT_SHORT_NAME_UQ' Title='PRODUCT_FRIENDLY_NAME_UQ' Level='1'>
      <ComponentRef Id='OurIERegistry' />
      <ComponentRef Id='OurFFRegistry' />
      <ComponentRef Id='OurSharedRegistry' />
      <ComponentRef Id='OurIEDirFiles' />
      <ComponentRef Id='OurFFDirFiles' />
      <ComponentRef Id='OurFFComponentsDirFiles' />
      <ComponentRef Id='OurFFLibDirFiles' />
      <ComponentRef Id='OurFFContentDirFiles' />
      <ComponentRef Id='OurSharedDirFiles' />
      <ComponentRef Id='OurSharedVersionedDirFiles' />
m4_foreach(~`LANG`~, I18N_LANGUAGES, ~`m4_dnl
      <ComponentRef Id='~`OurFF`~m4_underscore(LANG)~`DirFiles`~' />
`~)m4_dnl
    </Feature>
    <Condition Message='PRODUCT_FRIENDLY_NAME_UQ has already been updated.'>
      NOT NEWERVERSIONDETECTED
    </Condition>
    <InstallExecuteSequence>
      <RemoveExistingProducts After='InstallValidate'>UPGRADING</RemoveExistingProducts>
    </InstallExecuteSequence>
    <Property Id='UILevel'>1</Property>
    <Property Id='ALLUSERS'>1</Property>
    <!-- Set the icon in Add/Remove Programs -->
    <Property Id='ARPPRODUCTICON'>MainIcon</Property>
    <Icon Id='MainIcon' SourceFile="ui/common/icon_merged.ico" />
  </Product>
</Wix>
