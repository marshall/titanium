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

  <Product Id='$(var.OurNpapiProductId)' Name='PRODUCT_FRIENDLY_NAME_UQ'
    Language='1033' Version='PRODUCT_VERSION'
    Manufacturer='Google' UpgradeCode='D92DBAED-3E3E-4530-B30D-072D16C7DDD0'>
    <Package Description='PRODUCT_FRIENDLY_NAME_UQ'
      Comments='PRODUCT_FRIENDLY_NAME_UQ' Manufacturer='Google'
      InstallPrivileges='limited'
      InstallerVersion='200' Compressed='yes' />
    <Media Id='1' Cabinet='product.cab' EmbedCab='yes'
      CompressionLevel='high' />
    <Upgrade Id='D92DBAED-3E3E-4530-B30D-072D16C7DDD0'>
      <UpgradeVersion Property='UPGRADING' OnlyDetect='no'
        Minimum='0.0.0.0' IncludeMinimum='yes'
        Maximum='PRODUCT_VERSION' IncludeMaximum='no' />
      <UpgradeVersion Property='NEWERVERSIONDETECTED' OnlyDetect='yes'
        Minimum='PRODUCT_VERSION' IncludeMinimum='no' />
    </Upgrade>

    <Directory Id='TARGETDIR' Name='SourceDir'>
      <Directory Id='LocalAppDataFolder' Name='AppData'>
        <Directory Id='GoogleDir'  Name='Google'>
        <Directory Id='ChromeDir'  Name='Chrome'>
        <Directory Id='AppDir'     Name='Application'>
        <Directory Id='PluginsDir' Name='Plugins'>
          <Directory Id='OurNpapiDir' Name='PRODUCT_SHORT_NAME_UQ'>
          </Directory>
        </Directory>
        </Directory>
        </Directory>
        </Directory>
      </Directory>
    </Directory>

    <DirectoryRef Id='OurNpapiDir'>

      <Component Id='OurNpapiRegistry' Guid='$(var.OurComponentGUID_NpapiRegistry)'>
        <!-- Registry keys used by Google Update. -->
        <RegistryValue
          Root='HKCU' Key='Software\Google\Update\Clients\{00058422-BABE-4310-9B8B-B8DEB5D0B68A}'
          Action='write' Type='string'
          Name='pv' Value='PRODUCT_VERSION' />
      </Component>

      <Component Id='OurNpapiDirFiles' Guid='$(var.OurComponentGUID_NpapiFiles)'>
        <!-- For per-user installs, every WiX 'Component' needs a HKCU regkey,
             to use as the 'KeyPath'.  So handle one Google Update key here. -->
        <RegistryValue
          KeyPath='yes'
          Root='HKCU' Key='Software\Google\Update\Clients\{00058422-BABE-4310-9B8B-B8DEB5D0B68A}'
          Action='write' Type='string'
          Name='ap' Value='' />

        <File Id='npapi_dll' Name='PRODUCT_SHORT_NAME_UQ.dll' DiskId='1'
          Source="$(var.OurNpapiPath)/PRODUCT_SHORT_NAME_UQ.dll" />
m4_ifdef(~`DEBUG`~,~`m4_dnl
        <File Id='npapi_pdb' Name='PRODUCT_SHORT_NAME_UQ.pdb' DiskId='1'
          Source="$(var.OurNpapiPath)/PRODUCT_SHORT_NAME_UQ.pdb" />
`~)
m4_ifdef(~`USING_CCTESTS`~,~`m4_dnl
        <File Id='ie_notifier_test_exe' Name='notifier_test.exe' DiskId='1'
          Source="$(var.OurCommonPath)/notifier_test.exe" />
        <File Id='ie_ipc_test_exe' Name='ipc_test.exe' DiskId='1'
          Source="$(var.OurIpcTestPath)/ipc_test.exe" />
`~)
      </Component>

    </DirectoryRef>

    <Feature Id='PRODUCT_SHORT_NAME_UQ' Title='PRODUCT_FRIENDLY_NAME_UQ' Level='1'>
      <ComponentRef Id='OurNpapiRegistry' />
      <ComponentRef Id='OurNpapiDirFiles' />
    </Feature>
    <Condition Message='PRODUCT_FRIENDLY_NAME_UQ has already been updated.'>
      NOT NEWERVERSIONDETECTED
    </Condition>
    <InstallExecuteSequence>
      <RemoveExistingProducts After='InstallValidate'>UPGRADING</RemoveExistingProducts>
    </InstallExecuteSequence>
    <Property Id='UILevel'>1</Property>
    <!-- Ensure ALLUSERS has no value, making this a per-user install. -->
    <Property Id='ALLUSERS' Secure='yes' />
    <!-- Prevent showing this product in Add/Remove Programs. -->
    <Property Id='ARPSYSTEMCOMPONENT'>1</Property>
  </Product>
</Wix>
