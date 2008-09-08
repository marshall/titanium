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
content   PRODUCT_SHORT_NAME_UQ    chrome/chromeFiles/content/
overlay   chrome://browser/content/browser.xul    chrome://PRODUCT_SHORT_NAME_UQ/content/browser-overlay.xul
m4_foreach(~`LANG`~, I18N_LANGUAGES, ~`m4_dnl
locale    PRODUCT_SHORT_NAME_UQ    LANG   chrome/chromeFiles/locale/LANG/
`~)m4_dnl
