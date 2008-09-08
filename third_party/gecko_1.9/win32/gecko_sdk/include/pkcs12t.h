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

#ifndef _PKCS12T_H_
#define _PKCS12T_H_

#include "seccomon.h"
#include "secoid.h"
#include "cert.h"
#include "key.h"
#include "plarena.h"
#include "secpkcs7.h"
#include "secdig.h"	/* for SGNDigestInfo */

typedef enum {
  SECPKCS12TargetTokenNoCAs,		/* CA get loaded intothe fixed token,
					 * User certs go to target token */
  SECPKCS12TargetTokenIntermediateCAs,  /* User certs and intermediates go to
					 * target token, root certs got to
					 * fixed token */
  SECPKCS12TargetTokenAllCAs		/* All certs go to target token */
} SECPKCS12TargetTokenCAs;

/* PKCS12 Structures */
typedef struct SEC_PKCS12PFXItemStr SEC_PKCS12PFXItem;
typedef struct SEC_PKCS12MacDataStr SEC_PKCS12MacData;
typedef struct SEC_PKCS12AuthenticatedSafeStr SEC_PKCS12AuthenticatedSafe;
typedef struct SEC_PKCS12BaggageItemStr SEC_PKCS12BaggageItem;
typedef struct SEC_PKCS12BaggageStr SEC_PKCS12Baggage;
typedef struct SEC_PKCS12Baggage_OLDStr SEC_PKCS12Baggage_OLD;
typedef struct SEC_PKCS12ESPVKItemStr SEC_PKCS12ESPVKItem;
typedef struct SEC_PKCS12PVKSupportingDataStr SEC_PKCS12PVKSupportingData;
typedef struct SEC_PKCS12PVKAdditionalDataStr SEC_PKCS12PVKAdditionalData;
typedef struct SEC_PKCS12SafeContentsStr SEC_PKCS12SafeContents;
typedef struct SEC_PKCS12SafeBagStr SEC_PKCS12SafeBag;
typedef struct SEC_PKCS12PrivateKeyStr SEC_PKCS12PrivateKey;
typedef struct SEC_PKCS12PrivateKeyBagStr SEC_PKCS12PrivateKeyBag;
typedef struct SEC_PKCS12CertAndCRLBagStr SEC_PKCS12CertAndCRLBag;
typedef struct SEC_PKCS12CertAndCRLStr SEC_PKCS12CertAndCRL;
typedef struct SEC_PKCS12X509CertCRLStr SEC_PKCS12X509CertCRL;
typedef struct SEC_PKCS12SDSICertStr SEC_PKCS12SDSICert;
typedef struct SEC_PKCS12SecretStr SEC_PKCS12Secret;
typedef struct SEC_PKCS12SecretAdditionalStr SEC_PKCS12SecretAdditional;
typedef struct SEC_PKCS12SecretItemStr SEC_PKCS12SecretItem;
typedef struct SEC_PKCS12SecretBagStr SEC_PKCS12SecretBag;

typedef SECItem *(* SEC_PKCS12PasswordFunc)(SECItem *args);

/* PKCS12 types */

/* stores shrouded keys */
struct SEC_PKCS12BaggageStr
{
    PRArenaPool     *poolp;
    SEC_PKCS12BaggageItem **bags;

    int luggage_size;		/* used locally */
};

/* additional data to be associated with keys.	currently there
 * is nothing defined to be stored here.  allows future expansion.
 */
struct SEC_PKCS12PVKAdditionalDataStr
{
    PRArenaPool	*poolp;
    SECOidData	*pvkAdditionalTypeTag;	/* used locally */
    SECItem     pvkAdditionalType;
    SECItem     pvkAdditionalContent;
};

/* cert and other supporting data for private keys.  used
 * for both shrouded and non-shrouded keys.
 */
struct SEC_PKCS12PVKSupportingDataStr
{
    PRArenaPool		*poolp;
    SGNDigestInfo 	**assocCerts;
    SECItem		regenerable;
    SECItem         	nickname;
    SEC_PKCS12PVKAdditionalData     pvkAdditional;
    SECItem		pvkAdditionalDER;

    SECItem		uniNickName;
    /* used locally */
    int			nThumbs;
};

/* shrouded key structure.  supports only pkcs8 shrouding
 * currently.
 */
struct SEC_PKCS12ESPVKItemStr
{
    PRArenaPool *poolp;		/* used locally */
    SECOidData	*espvkTag;	/* used locally */
    SECItem	espvkOID;
    SEC_PKCS12PVKSupportingData espvkData;
    union
    {
	SECKEYEncryptedPrivateKeyInfo *pkcs8KeyShroud;
    } espvkCipherText;

    PRBool duplicate;	/* used locally */
    PRBool problem_cert; 	/* used locally */
    PRBool single_cert;		/* used locally */
    int nCerts;			/* used locally */
    SECItem derCert;		/* used locally */
};

/* generic bag store for the safe.  safeBagType identifies
 * the type of bag stored.
 */
struct SEC_PKCS12SafeBagStr
{
    PRArenaPool *poolp;
    SECOidData	*safeBagTypeTag;	/* used locally */
    SECItem     safeBagType;
    union
    {
	SEC_PKCS12PrivateKeyBag	*keyBag;
	SEC_PKCS12CertAndCRLBag *certAndCRLBag;
	SEC_PKCS12SecretBag     *secretBag;
    } safeContent;

    SECItem	derSafeContent;
    SECItem 	safeBagName;

    SECItem	uniSafeBagName;
};

/* stores private keys and certificates in a list.  each safebag
 * has an ID identifying the type of content stored.
 */
struct SEC_PKCS12SafeContentsStr
{
    PRArenaPool     	*poolp;
    SEC_PKCS12SafeBag	**contents;

    /* used for tracking purposes */
    int safe_size;
    PRBool old;
    PRBool swapUnicode;
    PRBool possibleSwapUnicode;
};

/* private key structure which holds encrypted private key and
 * supporting data including nickname and certificate thumbprint.
 */
struct SEC_PKCS12PrivateKeyStr
{
    PRArenaPool *poolp;
    SEC_PKCS12PVKSupportingData pvkData;
    SECKEYPrivateKeyInfo	pkcs8data;   /* borrowed from PKCS 8 */

    PRBool duplicate;	/* used locally */
    PRBool problem_cert;/* used locally */
    PRBool single_cert;	/* used locally */
    int nCerts;		/* used locally */
    SECItem derCert;	/* used locally */
};

/* private key bag, holds a (null terminated) list of private key
 * structures.
 */
struct SEC_PKCS12PrivateKeyBagStr
{
    PRArenaPool     *poolp;
    SEC_PKCS12PrivateKey 	**privateKeys;

    int bag_size;	/* used locally */
};

/* container to hold certificates.  currently supports x509
 * and sdsi certificates
 */
struct SEC_PKCS12CertAndCRLStr
{
    PRArenaPool     *poolp;
    SECOidData	    *BagTypeTag;    /* used locally */
    SECItem         BagID;
    union
    {
    	SEC_PKCS12X509CertCRL	*x509;
    	SEC_PKCS12SDSICert	*sdsi;
    } value;

    SECItem derValue;
    SECItem nickname;		/* used locally */
    PRBool duplicate;		/* used locally */
};

/* x509 certificate structure.	typically holds the der encoding
 * of the x509 certificate.  thumbprint contains a digest of the
 * certificate
 */
struct SEC_PKCS12X509CertCRLStr
{
    PRArenaPool     		*poolp;
    SEC_PKCS7ContentInfo	certOrCRL;
    SGNDigestInfo		thumbprint;

    SECItem *derLeafCert;	/* used locally */
};

/* sdsi certificate structure.	typically holds the der encoding
 * of the sdsi certificate.  thumbprint contains a digest of the
 * certificate
 */
struct SEC_PKCS12SDSICertStr
{
    PRArenaPool     *poolp;
    SECItem         value;
    SGNDigestInfo   thumbprint;
};

/* contains a null terminated list of certs and crls */
struct SEC_PKCS12CertAndCRLBagStr
{
    PRArenaPool     		*poolp;
    SEC_PKCS12CertAndCRL	**certAndCRLs;

    int bag_size;	/* used locally */
};

/* additional secret information.  currently no information
 * stored in this structure.
 */
struct SEC_PKCS12SecretAdditionalStr
{
    PRArenaPool     *poolp;
    SECOidData	    *secretTypeTag;         /* used locally */
    SECItem         secretAdditionalType;
    SECItem         secretAdditionalContent;
};

/* secrets container.  this will be used to contain currently
 * unspecified secrets.  (it's a secret)
 */
struct SEC_PKCS12SecretStr
{
    PRArenaPool     *poolp;
    SECItem	secretName;
    SECItem	value;
    SEC_PKCS12SecretAdditional	secretAdditional;

    SECItem	uniSecretName;
};

struct SEC_PKCS12SecretItemStr
{
    PRArenaPool     *poolp;
    SEC_PKCS12Secret	secret;
    SEC_PKCS12SafeBag	subFolder;
};    

/* a bag of secrets.  holds a null terminated list of secrets.
 */
struct SEC_PKCS12SecretBagStr
{
    PRArenaPool     	*poolp;
    SEC_PKCS12SecretItem	**secrets;

    int bag_size;	/* used locally */
};

struct SEC_PKCS12MacDataStr
{
    SGNDigestInfo	safeMac;
    SECItem		macSalt;
};

/* outer transfer unit */
struct SEC_PKCS12PFXItemStr
{
    PRArenaPool		*poolp;
    SEC_PKCS12MacData	macData;
    SEC_PKCS7ContentInfo	authSafe; 

    /* for compatibility with beta */
    PRBool		old;
    SGNDigestInfo 	old_safeMac;
    SECItem		old_macSalt;

    /* compatibility between platforms for unicode swapping */
    PRBool		swapUnicode;
};

struct SEC_PKCS12BaggageItemStr {
    PRArenaPool	    *poolp;
    SEC_PKCS12ESPVKItem	**espvks;
    SEC_PKCS12SafeBag	**unencSecrets;

    int nEspvks;
    int nSecrets; 
};
    
/* stores shrouded keys */
struct SEC_PKCS12Baggage_OLDStr
{
    PRArenaPool     *poolp;
    SEC_PKCS12ESPVKItem **espvks;

    int luggage_size;		/* used locally */
};

/* authenticated safe, stores certs, keys, and shrouded keys */
struct SEC_PKCS12AuthenticatedSafeStr
{
    PRArenaPool     *poolp;
    SECItem         version;
    SECOidData	    *transportTypeTag;	/* local not part of encoding*/
    SECItem         transportMode;
    SECItem         privacySalt;
    SEC_PKCS12Baggage	  baggage;
    SEC_PKCS7ContentInfo  *safe;

    /* used for beta compatibility */
    PRBool old;
    PRBool emptySafe;
    SEC_PKCS12Baggage_OLD old_baggage;
    SEC_PKCS7ContentInfo old_safe;
    PRBool swapUnicode;
};
#define SEC_PKCS12_PFX_VERSION		1		/* what we create */



/* PKCS 12 Templates */
extern const SEC_ASN1Template SEC_PKCS12PFXItemTemplate_OLD[];
extern const SEC_ASN1Template SEC_PKCS12AuthenticatedSafeTemplate_OLD[];
extern const SEC_ASN1Template SEC_PKCS12BaggageTemplate_OLD[];
extern const SEC_ASN1Template SEC_PKCS12PFXItemTemplate[];
extern const SEC_ASN1Template SEC_PKCS12MacDataTemplate[];
extern const SEC_ASN1Template SEC_PKCS12AuthenticatedSafeTemplate[];
extern const SEC_ASN1Template SEC_PKCS12BaggageTemplate[];
extern const SEC_ASN1Template SEC_PKCS12ESPVKItemTemplate[];
extern const SEC_ASN1Template SEC_PKCS12PVKSupportingDataTemplate[];
extern const SEC_ASN1Template SEC_PKCS12PVKAdditionalTemplate[];
extern const SEC_ASN1Template SEC_PKCS12SafeContentsTemplate_OLD[];
extern const SEC_ASN1Template SEC_PKCS12SafeContentsTemplate[];
extern const SEC_ASN1Template SEC_PKCS12SafeBagTemplate[];
extern const SEC_ASN1Template SEC_PKCS12PrivateKeyTemplate[];
extern const SEC_ASN1Template SEC_PKCS12PrivateKeyBagTemplate[];
extern const SEC_ASN1Template SEC_PKCS12CertAndCRLTemplate[];
extern const SEC_ASN1Template SEC_PKCS12CertAndCRLBagTemplate[];
extern const SEC_ASN1Template SEC_PKCS12X509CertCRLTemplate_OLD[];
extern const SEC_ASN1Template SEC_PKCS12X509CertCRLTemplate[];
extern const SEC_ASN1Template SEC_PKCS12SDSICertTemplate[];
extern const SEC_ASN1Template SEC_PKCS12SecretBagTemplate[];
extern const SEC_ASN1Template SEC_PKCS12SecretTemplate[];
extern const SEC_ASN1Template SEC_PKCS12SecretItemTemplate[];
extern const SEC_ASN1Template SEC_PKCS12SecretAdditionalTemplate[];
extern const SEC_ASN1Template SGN_DigestInfoTemplate[];
extern const SEC_ASN1Template SEC_PointerToPKCS12KeyBagTemplate[];
extern const SEC_ASN1Template SEC_PointerToPKCS12CertAndCRLBagTemplate[];
extern const SEC_ASN1Template SEC_PointerToPKCS12CertAndCRLBagTemplate_OLD[];
extern const SEC_ASN1Template SEC_PointerToPKCS12SecretBagTemplate[];
extern const SEC_ASN1Template SEC_PointerToPKCS12X509CertCRLTemplate_OLD[];
extern const SEC_ASN1Template SEC_PointerToPKCS12X509CertCRLTemplate[];
extern const SEC_ASN1Template SEC_PointerToPKCS12SDSICertTemplate[];
extern const SEC_ASN1Template SEC_PKCS12CodedSafeBagTemplate[];
extern const SEC_ASN1Template SEC_PKCS12CodedCertBagTemplate[];
extern const SEC_ASN1Template SEC_PKCS12CodedCertAndCRLBagTemplate[];
extern const SEC_ASN1Template SEC_PKCS12PVKSupportingDataTemplate_OLD[];
extern const SEC_ASN1Template SEC_PKCS12ESPVKItemTemplate_OLD[];
#endif
