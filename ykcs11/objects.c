#include "obj_types.h"
#include "objects.h"
#include <ykpiv.h>
#include <string.h>
#include <stdlib.h>
#include "openssl_utils.h"

#define IS_CERT(x) (((x) >= PIV_CERT_OBJ_X509_PIV_AUTH && (x) <  PIV_CERT_OBJ_LAST) ? CK_TRUE : CK_FALSE)

CK_RV get_doa(CK_OBJECT_HANDLE obj, CK_ATTRIBUTE_PTR template); // TODO: static?
CK_RV get_coa(CK_OBJECT_HANDLE obj, CK_ATTRIBUTE_PTR template);
CK_RV get_proa(CK_OBJECT_HANDLE obj, CK_ATTRIBUTE_PTR template);
CK_RV get_puoa(CK_OBJECT_HANDLE obj, CK_ATTRIBUTE_PTR template);

//TODO: this is mostly a snippet from OpenSC how to give credit?     Less and less so now
/* Must be in order, and one per enumerated PIV_OBJ */
static piv_obj_t piv_objects[] = {
  {PIV_DATA_OBJ_X509_PIV_AUTH, 1, 0, 0, "X.509 Certificate for PIV Authentication", 0, 0, get_doa, 0},
  {PIV_DATA_OBJ_X509_CARD_AUTH, 1, 0, 0, "X.509 Certificate for Card Authentication", 0, 0, get_doa, 1},
  {PIV_DATA_OBJ_X509_DS, 1, 0, 0, "X.509 Certificate for Digital Signature", 0, 0, get_doa, 2},
  {PIV_DATA_OBJ_X509_KM, 1, 0, 0, "X.509 Certificate for Key Management", 0, 0, get_doa, 3},
  {PIV_DATA_OBJ_CCC, 1, 0, 0, "Card Capability Container", 0, 0, get_doa, 4},
  {PIV_DATA_OBJ_CHUI, 1, 0, 0, "Card Holder Unique Identifier", 0, 0, get_doa, 5},
  {PIV_DATA_OBJ_CHF, 1, 1, 0, "Card Holder Fingerprints", 0, 0, get_doa, 6},
  {PIV_DATA_OBJ_SEC_OBJ, 1, 0, 0, "Security Object", 0, 0, get_doa, 7},
  {PIV_DATA_OBJ_CHFI, 1, 1, 0, "Cardholder Facial Images", 0, 0, get_doa, 8},
  {PIV_DATA_OBJ_PI, 1, 1, 0, "Printed Information", 0, 0, get_doa, 9},
  {PIV_DATA_OBJ_DISCOVERY, 1, 0, 0, "Discovery Object", 0, 0, get_doa, 10},
  {PIV_DATA_OBJ_HISTORY, 1, 0, 0, "Key History Object", 0, 0, get_doa, 11},
  {PIV_DATA_OBJ_RETIRED_X509_1, 1, 0, 0, "Retired X.509 Certificate for Key Management 1", 0, 0, get_doa, 12},
  {PIV_DATA_OBJ_RETIRED_X509_2, 1, 0, 0, "Retired X.509 Certificate for Key Management 2", 0, 0, get_doa, 13},
  {PIV_DATA_OBJ_RETIRED_X509_3, 1, 0, 0, "Retired X.509 Certificate for Key Management 3", 0, 0, get_doa, 14},
  {PIV_DATA_OBJ_RETIRED_X509_4, 1, 0, 0, "Retired X.509 Certificate for Key Management 4", 0, 0, get_doa, 15},
  {PIV_DATA_OBJ_RETIRED_X509_5, 1, 0, 0, "Retired X.509 Certificate for Key Management 5", 0, 0, get_doa, 16},
  {PIV_DATA_OBJ_RETIRED_X509_6, 1, 0, 0, "Retired X.509 Certificate for Key Management 6", 0, 0, get_doa, 17},
  {PIV_DATA_OBJ_RETIRED_X509_7, 1, 0, 0, "Retired X.509 Certificate for Key Management 7", 0, 0, get_doa, 18},
  {PIV_DATA_OBJ_RETIRED_X509_8, 1, 0, 0, "Retired X.509 Certificate for Key Management 8", 0, 0, get_doa, 19},
  {PIV_DATA_OBJ_RETIRED_X509_9, 1, 0, 0, "Retired X.509 Certificate for Key Management 9", 0, 0, get_doa, 20},
  {PIV_DATA_OBJ_RETIRED_X509_10, 1, 0, 0, "Retired X.509 Certificate for Key Management 10", 0, 0, get_doa, 21},
  {PIV_DATA_OBJ_RETIRED_X509_11, 1, 0, 0, "Retired X.509 Certificate for Key Management 11", 0, 0, get_doa, 22},
  {PIV_DATA_OBJ_RETIRED_X509_12, 1, 0, 0, "Retired X.509 Certificate for Key Management 12", 0, 0, get_doa, 23},
  {PIV_DATA_OBJ_RETIRED_X509_13, 1, 0, 0, "Retired X.509 Certificate for Key Management 13", 0, 0, get_doa, 24},
  {PIV_DATA_OBJ_RETIRED_X509_14, 1, 0, 0, "Retired X.509 Certificate for Key Management 14", 0, 0, get_doa, 25},
  {PIV_DATA_OBJ_RETIRED_X509_15, 1, 0, 0, "Retired X.509 Certificate for Key Management 15", 0, 0, get_doa, 26},
  {PIV_DATA_OBJ_RETIRED_X509_16, 1, 0, 0, "Retired X.509 Certificate for Key Management 16", 0, 0, get_doa, 27},
  {PIV_DATA_OBJ_RETIRED_X509_17, 1, 0, 0, "Retired X.509 Certificate for Key Management 17", 0, 0, get_doa, 28},
  {PIV_DATA_OBJ_RETIRED_X509_18, 1, 0, 0, "Retired X.509 Certificate for Key Management 18", 0, 0, get_doa, 29},
  {PIV_DATA_OBJ_RETIRED_X509_19, 1, 0, 0, "Retired X.509 Certificate for Key Management 19", 0, 0, get_doa, 30},
  {PIV_DATA_OBJ_RETIRED_X509_20, 1, 0, 0, "Retired X.509 Certificate for Key Management 20", 0, 0, get_doa, 31},
  {PIV_DATA_OBJ_IRIS_IMAGE, 1, 1, 0, "Cardholder Iris Images", 0, 0, get_doa, 32},
  {PIV_DATA_OBJ_BITGT, 1, 0, 0, "Biometric Information Templates Group Template", 0, 0, get_doa, 33},
  {PIV_DATA_OBJ_SM_SIGNER, 1, 0, 0, "Secure Messaging Certificate Signer", 0, 0, get_doa, 34},
  {PIV_DATA_OBJ_PC_REF_DATA, 1, 1, 0, "Pairing Code Reference Data Container", 0, 0, get_doa, 35},
  {PIV_DATA_OBJ_LAST, 1, 0, 0, "", 0, 0, NULL, 36},

  {PIV_CERT_OBJ_X509_PIV_AUTH, 1, 0, 0, "X.509 Certificate for PIV Authentication", 0, 0, get_coa, 0},
  {PIV_CERT_OBJ_X509_CARD_AUTH, 1, 0, 0, "X.509 Certificate for Card Authentication", 0, 0, get_coa, 1},
  {PIV_CERT_OBJ_X509_DS, 1, 0, 0, "X.509 Certificate for Digital Signature", 0, 0, get_coa, 2},
  {PIV_CERT_OBJ_X509_KM, 1, 0, 0, "X.509 Certificate for Key Management", 0, 0, get_coa, 3},
  {PIV_CERT_OBJ_LAST, 1, 0, 0, "", 0, 0, get_coa, 4},

  {PIV_PVTK_OBJ_PIV_AUTH, 1, 0, 0, "Private key for PIV Authentication", 0, 0, get_proa, 0},   // 9a
  {PIV_PVTK_OBJ_CARD_AUTH, 1, 0, 0, "Private key for Card Authentication", 0, 0, get_proa, 1}, // 9e
  {PIV_PVTK_OBJ_DS, 1, 0, 0, "Private key for Digital Signature", 0, 0, get_proa, 2},          // 9c
  {PIV_PVTK_OBJ_KM, 1, 0, 0, "Prrivate key for Key Management", 0, 0, get_proa, 3},            // 9d
  {PIV_PVTK_OBJ_LAST, 1, 0, 0, "", 0, 0, NULL, 4},

  {PIV_PUBK_OBJ_PIV_AUTH, 1, 0, 0, "Public key for PIV Authentication", 0, 0, get_puoa, 0},
  {PIV_PUBK_OBJ_CARD_AUTH, 1, 0, 0, "Public key for Card Authentication", 0, 0, get_puoa, 1},
  {PIV_PUBK_OBJ_DS, 1, 0, 0, "Public key for Digital Signature", 0, 0, get_puoa, 2},
  {PIV_PUBK_OBJ_KM, 1, 0, 0, "Public key for Key Management", 0, 0, get_puoa, 3},
  {PIV_PUBK_OBJ_LAST, 1, 0, 0, "", 0, 0, NULL, 4}
};

static piv_data_obj_t data_objects[] = {
  {"2.16.840.1.101.3.7.2.1.1",   3, "\x5F\xC1\x05", "\x01\x01"},
  {"2.16.840.1.101.3.7.2.5.0",   3, "\x5F\xC1\x01", "\x05\x00"},
  {"2.16.840.1.101.3.7.2.1.0",   3, "\x5F\xC1\x0A", "\x01\x00"},
  {"2.16.840.1.101.3.7.2.1.2",   3, "\x5F\xC1\x0B", "\x01\x02"},
  {"2.16.840.1.101.3.7.1.219.0", 3, "\x5F\xC1\x07", "\xDB\x00"},
  {"2.16.840.1.101.3.7.2.48.0",  3, "\x5F\xC1\x02", "\x30\x00"},
  {"2.16.840.1.101.3.7.2.96.16", 3, "\x5F\xC1\x03", "\x60\x10"},
  {"2.16.840.1.101.3.7.2.144.0", 3, "\x5F\xC1\x06", "\x90\x00"},
  {"2.16.840.1.101.3.7.2.96.48", 3, "\x5F\xC1\x08", "\x60\x30"},
  {"2.16.840.1.101.3.7.2.48.1",  3, "\x5F\xC1\x09", "\x30\x01"},
  {"2.16.840.1.101.3.7.2.96.80", 1, "\x7E",         "\x60\x50"},
  {"2.16.840.1.101.3.7.2.96.96", 3, "\x5F\xC1\x0C", "\x60\x60"},

/* 800-73-3, 21 new objects, 20 history certificates */
  {"2.16.840.1.101.3.7.2.16.1",  3, "\x5F\xC1\x0D", "\x10\x01"},
  {"2.16.840.1.101.3.7.2.16.2",  3, "\x5F\xC1\x0E", "\x10\x02"},
  {"2.16.840.1.101.3.7.2.16.3",  3, "\x5F\xC1\x0F", "\x10\x03"},
  {"2.16.840.1.101.3.7.2.16.4",  3, "\x5F\xC1\x10", "\x10\x04"},
  {"2.16.840.1.101.3.7.2.16.5",  3, "\x5F\xC1\x11", "\x10\x05"},
  {"2.16.840.1.101.3.7.2.16.7",  3, "\x5F\xC1\x13", "\x10\x07"},
  {"2.16.840.1.101.3.7.2.16.8",  3, "\x5F\xC1\x14", "\x10\x08"},
  {"2.16.840.1.101.3.7.2.16.9",  3, "\x5F\xC1\x15", "\x10\x09"},
  {"2.16.840.1.101.3.7.2.16.10", 3, "\x5F\xC1\x16", "\x10\x0A"},
  {"2.16.840.1.101.3.7.2.16.11", 3, "\x5F\xC1\x17", "\x10\x0B"},
  {"2.16.840.1.101.3.7.2.16.12", 3, "\x5F\xC1\x18", "\x10\x0C"},
  {"2.16.840.1.101.3.7.2.16.13", 3, "\x5F\xC1\x19", "\x10\x0D"},
  {"2.16.840.1.101.3.7.2.16.14", 3, "\x5F\xC1\x1A", "\x10\x0E"},
  {"2.16.840.1.101.3.7.2.16.15", 3, "\x5F\xC1\x1B", "\x10\x0F"},
  {"2.16.840.1.101.3.7.2.16.16", 3, "\x5F\xC1\x1C", "\x10\x10"},
  {"2.16.840.1.101.3.7.2.16.17", 3, "\x5F\xC1\x1D", "\x10\x11"},
  {"2.16.840.1.101.3.7.2.16.18", 3, "\x5F\xC1\x1E", "\x10\x12"},
  {"2.16.840.1.101.3.7.2.16.19", 3, "\x5F\xC1\x1F", "\x10\x13"},
  {"2.16.840.1.101.3.7.2.16.20", 3, "\x5F\xC1\x20", "\x10\x14"},
  {"2.16.840.1.101.3.7.2.16.21", 3, "\x5F\xC1\x21", "\x10\x15"},
  {"2.16.840.1.101.3.7.2.16.22", 2, "\x7F\x61",     "\x10\x16"},
  {"2.16.840.1.101.3.7.2.16.23", 3, "\x5F\xC1\x22", "\x10\x17"},
  {"2.16.840.1.101.3.7.2.16.24", 3, "\x5F\xC1\x23", "\x10\x18"},
  {"", 0, "", ""}
};

static piv_cert_obj_t cert_objects[] = {
  {NULL},
  {NULL},
  {NULL},
  {NULL},
  {NULL}
};

static piv_pvtk_obj_t pvtkey_objects[] = {
  {0},
  {0},
  {0},
  {0},
  {0}
};

static piv_pubk_obj_t pubkey_objects[] = {
  {0},
  {0},
  {0},
  {0},
  {0}
};


/*static void get_object_class(CK_OBJECT_HANDLE obj, CK_OBJECT_CLASS_PTR class) {
  if (obj >= 0 && obj < PIV_DATA_OBJ_LAST)
    *class = CKO_DATA;
  else if (obj > PIV_DATA_OBJ_LAST && obj < PIV_CERT_OBJ_LAST)
    *class = CKO_CERTIFICATE;
  else
    *class = CKO_VENDOR_DEFINED | CKO_DATA; // Invalid value
    }*/

/*static void get_object_label(CK_OBJECT_HANDLE obj, CK_UTF8CHAR_PTR label) {
  strcpy((char *)label, objects[obj].name);
}
*/

// Next two functions based off the code at
// https://github.com/m9aertner/oidConverter/blob/master/oid.c
// TODO: how to give credit? OR JUST STORE THE OID ALREADY ENCODED?
static void make_base128(unsigned long l, int first, CK_BYTE_PTR buf, CK_ULONG_PTR n) {
  if (l > 127)
    make_base128(l / 128, 0, buf, n);

  l %= 128;

  if (first)
    buf[(*n)++] = (CK_BYTE)l;
  else
    buf[(*n)++] = 0x80 | (CK_BYTE)l;
}

static void asn1_encode_oid(CK_CHAR_PTR oid, CK_BYTE_PTR asn1_oid, CK_ULONG_PTR len) {
  CK_CHAR_PTR tmp = strdup((char *)oid);
  CK_CHAR_PTR p = tmp;
  CK_BYTE_PTR q = NULL;
  CK_ULONG    n = 0;
  CK_BYTE     b = 0;
  CK_ULONG    l = 0;
  CK_ULONG    nodes;

  q = p;
  *len = 0;
  nodes = 1;
  while (*p != 0) {
    if (*p == '.')
      nodes++;
    p++;
  }

  n = 0;
  b = 0;
  p = q;
  while (n < nodes) {
    q = p;
    while (*p != 0) {
      if (*p == '.')
        break;
      p++;
    }

    l = 0;
    if (*p == '.') {
      *p = 0;
      l = (CK_ULONG) atoi((char *)q);
      q = p + 1;
      p = q;
    }
    else {
      l = (CK_ULONG) atoi((char *)q);
      q = p;
    }

    /* Digit is in l. */
    if (n == 0)
      b = 40 * ((CK_BYTE)l);
    else if (n == 1) {
      b += (CK_BYTE) l;
      asn1_oid[(*len)++] = b;
    }
    else {
      make_base128(l, 1, asn1_oid, len);
    }
    n++;
  }

  free(tmp);
}

/*static void get_object_oid(CK_OBJECT_HANDLE obj, CK_UTF8CHAR_PTR oid) {
  strcpy((char *)oid, objects[obj].oid);
}

static void get_object_certificate_type(CK_OBJECT_HANDLE obj, CK_CERTIFICATE_TYPE_PTR type) {
  if ((objects[obj].flags & PIV_OBJECT_TYPE_CERT))
      *type = CKC_X_509;
}

static void get_object_key_id(CK_OBJECT_HANDLE obj, CK_UTF8CHAR_PTR key_id) {
  memcpy((char *)key_id, objects[obj].containerid, 2);
}
*/

static CK_KEY_TYPE get_key_type(EVP_PKEY *key) {
  return do_get_key_type(key);
}

static CK_KEY_TYPE get_modulus_bits(EVP_PKEY *key) {
  return do_get_rsa_modulus_length(key);
}

static CK_RV get_public_key(EVP_PKEY *key, CK_BYTE_PTR data, CK_ULONG_PTR len) {
  return do_get_public_key(key, data, len);
}

/* Get data object attribute */
CK_RV get_doa(CK_OBJECT_HANDLE obj, CK_ATTRIBUTE_PTR template) {
  CK_BYTE_PTR data;
  CK_BYTE     tmp[64];
  CK_ULONG    len = 0;
  fprintf(stderr, "FOR DATA OBJECT %lu, I WANT ", obj);

  switch (template->type) {
  case CKA_CLASS:
    fprintf(stderr, "CLASS\n");
    len = 1;
    tmp[0] = CKO_DATA;
    data = tmp;
    break;

  case CKA_TOKEN:
    // Technically all these objects are token objects
    fprintf(stderr, "TOKEN\n");
    len = 1;
    tmp[0] = piv_objects[obj].token;
    data = tmp;
    break;

  case CKA_PRIVATE:
    fprintf(stderr, "PRIVATE\n");
    len = 1;
    tmp[0] = piv_objects[obj].private;
    data = tmp;
    break;

  case CKA_LABEL:
    fprintf(stderr, "LABEL\n");
    len = strlen(piv_objects[obj].label) + 1;
    data = piv_objects[obj].label;
    break;

  case CKA_APPLICATION:
    fprintf(stderr, "APPLICATION\n");
    len = strlen(piv_objects[obj].label) + 1;
    data = piv_objects[obj].label;
    break;

  case CKA_VALUE: // TODO: this can be done with -r and -d|-a
    fprintf(stderr, "VALUE TODO!!!\n");
    return CKR_FUNCTION_FAILED;

  case CKA_OBJECT_ID: // TODO: how about just storing the OID in DER ?
    // This only makes sense for data objects
    fprintf(stderr, "OID\n");
    strcpy((char *)tmp, data_objects[piv_objects[obj].sub_id].oid);
    asn1_encode_oid(tmp, tmp, &len);
    data = tmp;
    break;

  /* case CKA_CERTIFICATE_TYPE: */
  /*   fprintf(stderr, "CERTIFICATE TYPE\n"); */
  /*   len = 1; */
  /*   tmp[0] = CKC_X_509; // Support only X.509 certs */
  /*   data = tmp; */
  /*   break; */

//  case CKA_ISSUER:
//  case CKA_SERIAL_NUMBER:
  /* case CKA_KEY_TYPE: */
  /*   fprintf(stderr, "Return the key type TODO!!!\n"); */
  /*   return CKR_OK; */

  /* case CKA_SUBJECT: */
  /* case CKA_ID: */
  /*   fprintf(stderr, "ID\n"); */
  /*   len = data_objects[objects[obj].sub_id].tag_len; */
  /*   data = data_objects[objects[obj].sub_id].tag_value; */
  /*   break; */

  /* case CKA_SENSITIVE: */
  /* case CKA_ENCRYPT: */
  /* case CKA_DECRYPT: */
  /* case CKA_WRAP: */
  /* case CKA_UNWRAP: */
  /* case CKA_SIGN: */
  /* case CKA_SIGN_RECOVER: */
  /* case CKA_VERIFY: */
  /* case CKA_VERIFY_RECOVER: */
  /* case CKA_DERIVE: */
  /* case CKA_START_DATE: */
  /* case CKA_END_DATE: */
  /* case CKA_MODULUS: */
  /* case CKA_MODULUS_BITS: */
  /* case CKA_PUBLIC_EXPONENT: */
  /* case CKA_PRIVATE_EXPONENT: */
  /* case CKA_PRIME_1: */
  /* case CKA_PRIME_2: */
  /* case CKA_EXPONENT_1: */
  /* case CKA_EXPONENT_2: */
  /* case CKA_COEFFICIENT: */
  /* case CKA_PRIME: */
  /* case CKA_SUBPRIME: */
  /* case CKA_BASE: */
  /* case CKA_VALUE_BITS: */
  /* case CKA_VALUE_LEN: */
  /* case CKA_EXTRACTABLE: */
  /* case CKA_LOCAL: */
  /* case CKA_NEVER_EXTRACTABLE: */
  /* case CKA_ALWAYS_SENSITIVE: */
  case CKA_MODIFIABLE:
    fprintf(stderr, "MODIFIABLE\n");
    len = 1;
    tmp[0] = piv_objects[obj].modifiable;
    data = tmp;
    break;

  /* case CKA_VENDOR_DEFINED: */
  default:
    fprintf(stderr, "UNKNOWN ATTRIBUTE!!!!! %lx\n", template[0].type);
    template->ulValueLen = CK_UNAVAILABLE_INFORMATION;
    return CKR_ATTRIBUTE_TYPE_INVALID;
  }

  /* Just get the length */
  if (template->pValue == NULL_PTR) {
    template->ulValueLen = len; // TODO: define?
    return CKR_OK;
  }

  /* Actually get the attribute */
  if (template->ulValueLen < len)
    return CKR_BUFFER_TOO_SMALL;

  template->ulValueLen = len;
  memcpy(template->pValue, data, len);

  return CKR_OK;

}

/* Get certificate object attribute */
CK_RV get_coa(CK_OBJECT_HANDLE obj, CK_ATTRIBUTE_PTR template) {
  CK_BYTE_PTR data;
  CK_BYTE     tmp[64];
  CK_ULONG    len = 0;
  fprintf(stderr, "FOR CERTIFICATE OBJECT %lu, I WANT ", obj);

  switch (template->type) { // TODO: is this needed here? or is it enough ot have one a "level" above?
  case CKA_CLASS:
    fprintf(stderr, "CLASS\n");
    len = 1;
    tmp[0] = CKO_CERTIFICATE;
    data = tmp;
    break;

  case CKA_TOKEN:
    // Technically all these objects are token objects
    fprintf(stderr, "TOKEN\n");
    len = 1;
    tmp[0] = piv_objects[obj].token;
    data = tmp;
    break;

  case CKA_PRIVATE:
    fprintf(stderr, "PRIVATE\n");
    len = 1;
    tmp[0] = piv_objects[obj].private;
    data = tmp;
    break;

  case CKA_LABEL:
    fprintf(stderr, "LABEL\n");
    len = strlen(piv_objects[obj].label) + 1;
    data = piv_objects[obj].label;
    break;

  /* case CKA_APPLICATION: */
  /*   fprintf(stderr, "APPLICATION\n"); */
  /*   len = strlen(objects[obj].label) + 1; */
  /*   data = objects[obj].label; */
  /*   break; */

  case CKA_VALUE:
    fprintf(stderr, "VALUE TODO\n");
    return CKR_FUNCTION_FAILED;

  /* case CKA_OBJECT_ID: // TODO: how about just storing the OID in DER ? */
  /*   // This only makes sense for data objects */
  /*   fprintf(stderr, "OID\n"); */
  /*   strcpy((char *)tmp, certificate_objects[objects[obj].sub_id].oid); */
  /*   asn1_encode_oid(tmp, tmp, &len); */
  /*   data = tmp; */
  /*   break; */

  case CKA_CERTIFICATE_TYPE:
    fprintf(stderr, "CERTIFICATE TYPE\n");
    len = 1;
    tmp[0] = CKC_X_509; // Support only X.509 certs
    data = tmp;
    break;

  case CKA_ISSUER:
    fprintf(stderr, "ISSUER TODO\n"); // Default empty
    return CKR_FUNCTION_FAILED;

  case CKA_SERIAL_NUMBER:
    fprintf(stderr, "SERIAL NUMBER TODO\n"); // Default empty
    return CKR_FUNCTION_FAILED;

  /* case CKA_KEY_TYPE: */
  /*   fprintf(stderr, "Return the key type TODO!!!\n"); */
  /*   return CKR_OK; */

  case CKA_SUBJECT:
    fprintf(stderr, "SUBJECT TODO\n"); // Required
    return CKR_FUNCTION_FAILED;

  case CKA_ID:
    fprintf(stderr, "ID\n");
    len = 1;
    tmp[0] = piv_objects[obj].sub_id;
    data = tmp;
    break;

  /* case CKA_SENSITIVE: */
  /* case CKA_ENCRYPT: */
  /* case CKA_DECRYPT: */
  /* case CKA_WRAP: */
  /* case CKA_UNWRAP: */
  /* case CKA_SIGN: */
  /* case CKA_SIGN_RECOVER: */
  /* case CKA_VERIFY: */
  /* case CKA_VERIFY_RECOVER: */
  /* case CKA_DERIVE: */
  case CKA_START_DATE:
    fprintf(stderr, "START DATE TODO\n"); // Default empty
    return CKR_FUNCTION_FAILED;

  case CKA_END_DATE:
    fprintf(stderr, "END DATE TODO\n"); // Default empty
    return CKR_FUNCTION_FAILED;

  /* case CKA_MODULUS: */
  /* case CKA_MODULUS_BITS: */
  /* case CKA_PUBLIC_EXPONENT: */
  /* case CKA_PRIVATE_EXPONENT: */
  /* case CKA_PRIME_1: */
  /* case CKA_PRIME_2: */
  /* case CKA_EXPONENT_1: */
  /* case CKA_EXPONENT_2: */
  /* case CKA_COEFFICIENT: */
  /* case CKA_PRIME: */
  /* case CKA_SUBPRIME: */
  /* case CKA_BASE: */
  /* case CKA_VALUE_BITS: */
  /* case CKA_VALUE_LEN: */
  /* case CKA_EXTRACTABLE: */
  /* case CKA_LOCAL: */
  /* case CKA_NEVER_EXTRACTABLE: */
  /* case CKA_ALWAYS_SENSITIVE: */
  case CKA_MODIFIABLE:
    fprintf(stderr, "MODIFIABLE\n");
    len = 1;
    tmp[0] = piv_objects[obj].modifiable;
    data = tmp;
    break;

  /* case CKA_VENDOR_DEFINED: */
  default: // TODO: there are other attributes for a (x509) certificate
    fprintf(stderr, "UNKNOWN ATTRIBUTE!!!!! %lx\n", template[0].type);
    template->ulValueLen = CK_UNAVAILABLE_INFORMATION;
    return CKR_ATTRIBUTE_TYPE_INVALID;
  }

  /* Just get the length */
  if (template->pValue == NULL_PTR) {
    template->ulValueLen = len; // TODO: define?
    return CKR_OK;
  }

  /* Actually get the attribute */
  if (template->ulValueLen < len)
    return CKR_BUFFER_TOO_SMALL;

  template->ulValueLen = len;
  memcpy(template->pValue, data, len);

  return CKR_OK;

}

/* Get private key object attribute */
CK_RV get_proa(CK_OBJECT_HANDLE obj, CK_ATTRIBUTE_PTR template) {
  CK_BYTE_PTR data;
  CK_BYTE     b_tmp[1024];
  CK_ULONG    ul_tmp; // TODO: fix elsewhere too
  CK_ULONG    len = 0;
  fprintf(stderr, "FOR PRIVATE KEY OBJECT %lu, I WANT ", obj);

  switch (template->type) {
  case CKA_CLASS:
    fprintf(stderr, "CLASS\n");
    len = sizeof(CK_ULONG);
    ul_tmp = CKO_PRIVATE_KEY;
    data = (CK_BYTE_PTR) &ul_tmp;
    break;

  case CKA_TOKEN:
    // Technically all these objects are token objects
    fprintf(stderr, "TOKEN\n");
    len = sizeof(CK_BBOOL);
    b_tmp[0] = piv_objects[obj].token;
    data = b_tmp;
    break;

  case CKA_PRIVATE:
    fprintf(stderr, "PRIVATE\n");
    len = sizeof(CK_BBOOL);
    b_tmp[0] = piv_objects[obj].private;
    data = b_tmp;
    break;

  case CKA_LABEL:
    fprintf(stderr, "LABEL\n");
    len = strlen(piv_objects[obj].label) + 1;
    data = piv_objects[obj].label;
    break;

  /* case CKA_APPLICATION: */
  /*   fprintf(stderr, "APPLICATION\n"); */
  /*   len = strlen(objects[obj].label) + 1; */
  /*   data = objects[obj].label; */
  /*   break; */

//  case CKA_VALUE: // TODO: this can be done with -r and -d|-a
  /* case CKA_OBJECT_ID: // TODO: how about just storing the OID in DER ? */
  /*   // This only makes sense for data objects */
  /*   fprintf(stderr, "OID\n"); */
  /*   strcpy((char *)tmp, pvtkey_objects[objects[obj].sub_id].oid); */
  /*   asn1_encode_oid(tmp, tmp, &len); */
  /*   data = tmp; */
  /*   break; */

  /* case CKA_CERTIFICATE_TYPE: */
  /*   fprintf(stderr, "CERTIFICATE TYPE\n"); */
  /*   len = 1; */
  /*   tmp[0] = CKC_X_509; // Support only X.509 certs */
  /*   data = tmp; */
  /*   break; */

//  case CKA_ISSUER:
//  case CKA_SERIAL_NUMBER:
  case CKA_KEY_TYPE:
    fprintf(stderr, "KEY TYPE\n");
    len = sizeof(CK_ULONG);
    ul_tmp = get_key_type(pubkey_objects[piv_objects[obj].sub_id].data); // Getting the info from the pubk
    if (ul_tmp == CKK_VENDOR_DEFINED)
      return CKR_FUNCTION_FAILED;
    data = (CK_BYTE_PTR) &ul_tmp;
    break;

  case CKA_SUBJECT:
    fprintf(stderr, "SUBJECT TODO\n"); // Default empty
    return CKR_FUNCTION_FAILED;

  case CKA_ID:
    fprintf(stderr, "ID\n");
    len = sizeof(CK_BYTE);
    ul_tmp = piv_objects[obj].sub_id;
    data = (CK_BYTE_PTR) &ul_tmp;
    break;

  case CKA_SENSITIVE:
    fprintf(stderr, "SENSITIVE TODO\n"); // Default empty
    return CKR_FUNCTION_FAILED;

  /* case CKA_ENCRYPT: */
  case CKA_DECRYPT:
    fprintf(stderr, "DECRYPT TODO\n"); // Default empty
    return CKR_FUNCTION_FAILED;

  /* case CKA_WRAP: */
  case CKA_UNWRAP:
    fprintf(stderr, "UNWRAP TODO\n"); // Default empty
    return CKR_FUNCTION_FAILED;

  case CKA_SIGN:
    fprintf(stderr, "SIGN TODO\n"); // Default empty
    return CKR_FUNCTION_FAILED;

  case CKA_SIGN_RECOVER:
    fprintf(stderr, "SIGN RECOVER TODO\n"); // Default empty
    return CKR_FUNCTION_FAILED;

  /* case CKA_VERIFY: */
  /* case CKA_VERIFY_RECOVER: */
  case CKA_DERIVE:
    fprintf(stderr, "DERIVE TODO\n"); // Default false
    return CKR_FUNCTION_FAILED;

  case CKA_START_DATE:
    fprintf(stderr, "START DATE TODO\n"); // Default empty
    return CKR_FUNCTION_FAILED;

  case CKA_END_DATE:
    fprintf(stderr, "END DATE TODO\n"); // Default empty
    return CKR_FUNCTION_FAILED;

    /*case CKA_MODULUS:*/
  case CKA_EC_POINT:
    // We're trying to get the key length, get the ec point of the PUBLIC key
    // TODO: or just give an error and explicitly fetch the pubk len when needed
    fprintf(stderr, "EC_POINT\n");
    len = sizeof(b_tmp);
    if (get_public_key(pubkey_objects[piv_objects[obj].sub_id].data, b_tmp, &len) != CKR_OK)
      return CKR_FUNCTION_FAILED;
    data = b_tmp;
    break;
    
  case CKA_MODULUS_BITS:
    fprintf(stderr, "MODULUS BITS\n");
    len = sizeof(CK_ULONG);
    ul_tmp = get_modulus_bits(pubkey_objects[piv_objects[obj].sub_id].data); // Getting the info from the pubk
    if (ul_tmp == 0)
      return CKR_FUNCTION_FAILED;
    data = (CK_BYTE_PTR) &ul_tmp;
    break;
    
  /* case CKA_PUBLIC_EXPONENT: */
  /* case CKA_PRIVATE_EXPONENT: */
  /* case CKA_PRIME_1: */
  /* case CKA_PRIME_2: */
  /* case CKA_EXPONENT_1: */
  /* case CKA_EXPONENT_2: */
  /* case CKA_COEFFICIENT: */
  /* case CKA_PRIME: */
  /* case CKA_SUBPRIME: */
  /* case CKA_BASE: */
  /* case CKA_VALUE_BITS: */
  /* case CKA_VALUE_LEN: */
  /* case CKA_EXTRACTABLE: */
  case CKA_LOCAL:
    fprintf(stderr, "LOCAL TODO\n"); // Required
    return CKR_FUNCTION_FAILED;

  /* case CKA_NEVER_EXTRACTABLE: */
  /* case CKA_ALWAYS_SENSITIVE: */
  case CKA_MODIFIABLE:
    fprintf(stderr, "MODIFIABLE\n");
    len = sizeof(CK_BBOOL);
    b_tmp[0] = piv_objects[obj].modifiable;
    data = b_tmp;
    break;

    /*case CKA_VENDOR_DEFINED:*/
  default:
    fprintf(stderr, "UNKNOWN ATTRIBUTE!!!!! %lx\n", template[0].type); // TODO: there are other parameters for public keys, plus there is more if the key is RSA
    template->ulValueLen = CK_UNAVAILABLE_INFORMATION;
    return CKR_ATTRIBUTE_TYPE_INVALID;
  }

  /* Just get the length */
  if (template->pValue == NULL_PTR) {
    template->ulValueLen = len; // TODO: define?
    return CKR_OK;
  }

  /* Actually get the attribute */
  if (template->ulValueLen < len)
    return CKR_BUFFER_TOO_SMALL;

  template->ulValueLen = len;
  memcpy(template->pValue, data, len);

  return CKR_OK;

}

/* Get public key object attribute */
CK_RV get_puoa(CK_OBJECT_HANDLE obj, CK_ATTRIBUTE_PTR template) {
  CK_BYTE_PTR data;
  CK_BYTE     tmp[64];
  CK_ULONG    len = 0;
  fprintf(stderr, "FOR PUBLIC KEY OBJECT %lu, I WANT ", obj);

  switch (template->type) {
  case CKA_CLASS:
    fprintf(stderr, "CLASS\n");
    len = 1;
    tmp[0] = CKO_PUBLIC_KEY;
    data = tmp;
    break;

  case CKA_TOKEN:
    // Technically all these objects are token objects
    fprintf(stderr, "TOKEN\n");
    len = 1;
    tmp[0] = piv_objects[obj].token;
    data = tmp;
    break;

  case CKA_PRIVATE:
    fprintf(stderr, "PRIVATE\n");
    len = 1;
    tmp[0] = piv_objects[obj].private;
    data = tmp;
    break;

  case CKA_LABEL:
    fprintf(stderr, "LABEL\n");
    len = strlen(piv_objects[obj].label) + 1;
    data = piv_objects[obj].label;
    break;

  /* case CKA_APPLICATION: */
  /*   fprintf(stderr, "APPLICATION\n"); */
  /*   len = strlen(objects[obj].label) + 1; */
  /*   data = objects[obj].label; */
  /*   break; */

//  case CKA_VALUE: // TODO: this can be done with -r and -d|-a
  /* case CKA_OBJECT_ID: // TODO: how about just storing the OID in DER ? */
  /*   // This only makes sense for data objects */
  /*   fprintf(stderr, "OID\n"); */
  /*   strcpy((char *)tmp, pubkey_objects[objects[obj].sub_id].oid); */
  /*   asn1_encode_oid(tmp, tmp, &len); */
  /*   data = tmp; */
  /*   break; */

  /* case CKA_CERTIFICATE_TYPE: */
  /*   fprintf(stderr, "CERTIFICATE TYPE\n"); */
  /*   len = 1; */
  /*   tmp[0] = CKC_X_509; // Support only X.509 certs */
  /*   data = tmp; */
  /*   break; */

//  case CKA_ISSUER:
//  case CKA_SERIAL_NUMBER:
  case CKA_KEY_TYPE:
    fprintf(stderr, "KEY TYPE TODO\n");
    return CKR_FUNCTION_FAILED;

  case CKA_SUBJECT:
    fprintf(stderr, "SUBJECT TODO\n"); // Default empty
    return CKR_FUNCTION_FAILED;

  case CKA_ID:
    fprintf(stderr, "ID\n");
    len = 1;
    tmp[0] = piv_objects[obj].sub_id;
    data = tmp;
    break;

  /* case CKA_SENSITIVE: */
  case CKA_ENCRYPT:
    fprintf(stderr, "ENCRYPT TODO\n"); // Required
    return CKR_FUNCTION_FAILED;

  case CKA_DECRYPT:
    fprintf(stderr, "DECRYPT TODO\n"); // Required
    return CKR_FUNCTION_FAILED;

  case CKA_WRAP:
    fprintf(stderr, "WRAP TODO\n"); // Required
    return CKR_FUNCTION_FAILED;

  /* case CKA_UNWRAP: */
  /* case CKA_SIGN: */
  /* case CKA_SIGN_RECOVER: */
  /* case CKA_VERIFY: */
  /* case CKA_VERIFY_RECOVER: */
  case CKA_DERIVE:
    fprintf(stderr, "DERIVE TODO\n"); // Defaul false
    return CKR_FUNCTION_FAILED;

  case CKA_START_DATE:
    fprintf(stderr, "START DATE TODO\n"); // Default empty
    return CKR_FUNCTION_FAILED;

  case CKA_END_DATE:
    fprintf(stderr, "END DATE TODO\n"); // Default empty
    return CKR_FUNCTION_FAILED;
  /* case CKA_MODULUS: */
  /* case CKA_MODULUS_BITS: */
  /* case CKA_PUBLIC_EXPONENT: */
  /* case CKA_PRIVATE_EXPONENT: */
  /* case CKA_PRIME_1: */
  /* case CKA_PRIME_2: */
  /* case CKA_EXPONENT_1: */
  /* case CKA_EXPONENT_2: */
  /* case CKA_COEFFICIENT: */
  /* case CKA_PRIME: */
  /* case CKA_SUBPRIME: */
  /* case CKA_BASE: */
  /* case CKA_VALUE_BITS: */
  /* case CKA_VALUE_LEN: */
  /* case CKA_EXTRACTABLE: */
  case CKA_LOCAL:
    fprintf(stderr, "LOCAL TODO\n"); // Required
    return CKR_FUNCTION_FAILED;

  /* case CKA_NEVER_EXTRACTABLE: */
  /* case CKA_ALWAYS_SENSITIVE: */
  case CKA_MODIFIABLE:
    fprintf(stderr, "MODIFIABLE\n");
    len = 1;
    tmp[0] = piv_objects[obj].modifiable;
    data = tmp;
    break;

  /* case CKA_VENDOR_DEFINED: */
  default:
    fprintf(stderr, "UNKNOWN ATTRIBUTE!!!!! %lx\n", template[0].type); // TODO: there are other parameters for public keys
    template->ulValueLen = CK_UNAVAILABLE_INFORMATION;
    return CKR_ATTRIBUTE_TYPE_INVALID;
  }

  /* Just get the length */
  if (template->pValue == NULL_PTR) {
    template->ulValueLen = len; // TODO: define?
    return CKR_OK;
  }

  /* Actually get the attribute */
  if (template->ulValueLen < len)
    return CKR_BUFFER_TOO_SMALL;

  template->ulValueLen = len;
  memcpy(template->pValue, data, len);

  return CKR_OK;

}

CK_ULONG piv_2_ykpiv(piv_obj_id_t id) {
  // TODO: add retired keys
  switch(id) {
  case PIV_CERT_OBJ_X509_PIV_AUTH:
    return YKPIV_OBJ_AUTHENTICATION;
    
  case PIV_CERT_OBJ_X509_CARD_AUTH:
    return YKPIV_OBJ_CARD_AUTH;
    
  case PIV_CERT_OBJ_X509_DS:
    return YKPIV_OBJ_SIGNATURE;
    
  case PIV_CERT_OBJ_X509_KM:
    return YKPIV_OBJ_KEY_MANAGEMENT;

  case PIV_PVTK_OBJ_PIV_AUTH:
    return YKPIV_KEY_AUTHENTICATION;
    
  case PIV_PVTK_OBJ_CARD_AUTH:
    return YKPIV_KEY_CARDAUTH;
    
  case PIV_PVTK_OBJ_DS:
    return YKPIV_KEY_SIGNATURE;
    
  case PIV_PVTK_OBJ_KM:
    return YKPIV_KEY_KEYMGM;

  default:
    return 0ul;
  }
}

CK_RV get_attribute(ykcs11_session_t *s, CK_OBJECT_HANDLE obj, CK_ATTRIBUTE_PTR template) {
  CK_ULONG i;

  for (i = 0; i < s->slot->token->n_objects; i++)
    if (s->slot->token->objects[i] == obj) {
      return piv_objects[obj].get_attribute(obj, template);
    }

  return CKR_OBJECT_HANDLE_INVALID;
}

CK_BBOOL attribute_match(ykcs11_session_t *s, CK_OBJECT_HANDLE obj, CK_ATTRIBUTE_PTR attribute) {

  CK_ATTRIBUTE to_match;
  CK_BYTE_PTR data;

  // Get the size first
  to_match.type = attribute->type;
  to_match.pValue = NULL;
  to_match.ulValueLen = 0;

  if (get_attribute(s, obj, &to_match) != CKR_OK)
    return CK_FALSE;

  if (to_match.ulValueLen != attribute->ulValueLen)
    return CK_FALSE;

  // Allocate space for the attribute
  data = malloc(to_match.ulValueLen);
  if (data == NULL)
    return CK_FALSE;

  // Retrieve the attribute
  to_match.pValue = data;
  if (get_attribute(s, obj, &to_match) != CKR_OK) {
    free(data);
    data = NULL;
    return CK_FALSE;
  }

  // Compare the attributes
  if (memcmp(attribute->pValue, to_match.pValue, to_match.ulValueLen) != 0) {
    free(data);
    data = NULL;
    return CK_FALSE;
  }

  free(data);
  data = NULL;

  return CK_TRUE;
}

CK_RV get_available_certificate_ids(ykcs11_session_t *s, piv_obj_id_t *cert_ids, CK_ULONG n_certs) {
  CK_ULONG i, j;

  j = 0;
  for (i = 0; i < s->slot->token->n_objects; i++)
    if (IS_CERT(s->slot->token->objects[i]) == CK_TRUE)
      cert_ids[j++] = s->slot->token->objects[i];

  fprintf(stderr, "Just to check: %lu %lu\n", j, n_certs);

  return CKR_OK;
}

CK_RV store_cert(piv_obj_id_t cert_id, CK_BYTE_PTR data, CK_ULONG len) {

  CK_RV rv;

  // Store the certificate as an object
  rv = do_store_cert(data, len, &cert_objects[piv_objects[cert_id].sub_id].data);
  if (rv != CKR_OK)
    return rv;

  // Extract and store the public key as an object
  rv = do_store_pubk(cert_objects[piv_objects[cert_id].sub_id].data, &pubkey_objects[piv_objects[cert_id].sub_id].data);

  return CKR_OK;
}

