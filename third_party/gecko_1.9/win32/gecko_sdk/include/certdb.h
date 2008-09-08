/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Netscape security libraries.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1994-2000
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef _CERTDB_H_
#define _CERTDB_H_


/* common flags for all types of certificates */
#define CERTDB_VALID_PEER	(1<<0)
#define CERTDB_TRUSTED		(1<<1)
#define CERTDB_SEND_WARN	(1<<2)
#define CERTDB_VALID_CA		(1<<3)
#define CERTDB_TRUSTED_CA	(1<<4) /* trusted for issuing server certs */
#define CERTDB_NS_TRUSTED_CA	(1<<5)
#define CERTDB_USER		(1<<6)
#define CERTDB_TRUSTED_CLIENT_CA (1<<7) /* trusted for issuing client certs */
#define CERTDB_INVISIBLE_CA	(1<<8) /* don't show in UI */
#define CERTDB_GOVT_APPROVED_CA	(1<<9) /* can do strong crypto in export ver */


SEC_BEGIN_PROTOS

CERTSignedCrl *
SEC_FindCrlByKey(CERTCertDBHandle *handle, SECItem *crlKey, int type);

CERTSignedCrl *
SEC_FindCrlByName(CERTCertDBHandle *handle, SECItem *crlKey, int type);

CERTSignedCrl *
SEC_FindCrlByDERCert(CERTCertDBHandle *handle, SECItem *derCrl, int type);

PRBool
SEC_CertNicknameConflict(char *nickname, SECItem *derSubject,
			 CERTCertDBHandle *handle);
CERTSignedCrl *
SEC_NewCrl(CERTCertDBHandle *handle, char *url, SECItem *derCrl, int type);

SECStatus
SEC_DeletePermCRL(CERTSignedCrl *crl);


SECStatus
SEC_LookupCrls(CERTCertDBHandle *handle, CERTCrlHeadNode **nodes, int type);

SECStatus 
SEC_DestroyCrl(CERTSignedCrl *crl);

CERTSignedCrl* SEC_DupCrl(CERTSignedCrl* acrl);

SECStatus
CERT_AddTempCertToPerm(CERTCertificate *cert, char *nickname,
		       CERTCertTrust *trust);

SECStatus SEC_DeletePermCertificate(CERTCertificate *cert);

PRBool
SEC_CrlIsNewer(CERTCrl *inNew, CERTCrl *old);

SECCertTimeValidity
SEC_CheckCrlTimes(CERTCrl *crl, PRTime t);

#ifdef notdef
/*
** Add a DER encoded certificate to the permanent database.
**	"derCert" is the DER encoded certificate.
**	"nickname" is the nickname to use for the cert
**	"trust" is the trust parameters for the cert
*/
SECStatus SEC_AddPermCertificate(PCERTCertDBHandle *handle, SECItem *derCert,
				char *nickname, PCERTCertTrust *trust);

certDBEntryCert *
SEC_FindPermCertByKey(PCERTCertDBHandle *handle, SECItem *certKey);

certDBEntryCert
*SEC_FindPermCertByName(PCERTCertDBHandle *handle, SECItem *name);

SECStatus SEC_OpenPermCertDB(PCERTCertDBHandle *handle,
			     PRBool readOnly,
			     PCERTDBNameFunc namecb,
			     void *cbarg);


typedef SECStatus (PR_CALLBACK * PermCertCallback)(PCERTCertificate *cert,
                                                   SECItem *k, void *pdata);
/*
** Traverse the entire permanent database, and pass the certs off to a
** user supplied function.
**	"certfunc" is the user function to call for each certificate
**	"udata" is the user's data, which is passed through to "certfunc"
*/
SECStatus
PCERT_TraversePermCerts(PCERTCertDBHandle *handle,
		      PermCertCallback certfunc,
		      void *udata );

SECStatus
SEC_AddTempNickname(PCERTCertDBHandle *handle, char *nickname, SECItem *certKey);

SECStatus
SEC_DeleteTempNickname(PCERTCertDBHandle *handle, char *nickname);


PRBool
SEC_CertDBKeyConflict(SECItem *derCert, PCERTCertDBHandle *handle);

SECStatus
SEC_GetCrlTimes(PCERTCrl *dates, PRTime *notBefore, PRTime *notAfter);

PCERTSignedCrl *
SEC_AddPermCrlToTemp(PCERTCertDBHandle *handle, certDBEntryRevocation *entry);

SECStatus
SEC_DeleteTempCrl(PCERTSignedCrl *crl);


SECStatus
SEC_CheckKRL(PCERTCertDBHandle *handle,SECKEYLowPublicKey *key,
	     PCERTCertificate *rootCert, int64 t, void *wincx);

SECStatus
SEC_CheckCRL(PCERTCertDBHandle *handle,PCERTCertificate *cert,
	     PCERTCertificate *caCert, int64 t, void *wincx);

SECStatus
SEC_CrlReplaceUrl(PCERTSignedCrl *crl,char *url);

/* Compare two certificate validity structures and return which cert should be
** preferred, based first on newer notAfter, then on newer notBefore.
*/
CERTCompareValidityStatus
CERT_CompareValidityTimes(CERTValidity* val_a, CERTValidity* val_b);

#endif

SEC_END_PROTOS

#endif /* _CERTDB_H_ */
