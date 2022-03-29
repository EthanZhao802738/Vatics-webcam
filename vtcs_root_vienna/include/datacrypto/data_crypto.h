/*
 *******************************************************************************
 *  Copyright (c) 2010-2015 VATICS Inc. All rights reserved.
 *
 *  +-----------------------------------------------------------------+
 *  | THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY ONLY BE USED |
 *  | AND COPIED IN ACCORDANCE WITH THE TERMS AND CONDITIONS OF SUCH  |
 *  | A LICENSE AND WITH THE INCLUSION OF THE THIS COPY RIGHT NOTICE. |
 *  | THIS SOFTWARE OR ANY OTHER COPIES OF THIS SOFTWARE MAY NOT BE   |
 *  | PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY OTHER PERSON. THE   |
 *  | OWNERSHIP AND TITLE OF THIS SOFTWARE IS NOT TRANSFERRED.        |
 *  |                                                                 |
 *  | THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT   |
 *  | ANY PRIOR NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY |
 *  | VATICS INC.                                                     |
 *  +-----------------------------------------------------------------+
 *
 *******************************************************************************
 */

#ifndef DATA_CRYPTO
#define DATA_CRYPTO

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DataCrypto operation flag
 */
typedef enum vmf_dce_op_flags
{
	VMF_DCE_OP_ENCRYPT = 0,
	VMF_DCE_OP_DECRYPT = 1,
	VMF_DCE_OP_HASH = 2
} VMF_DCE_OP_FLAG;

/*
 * DataCrypto encrypto mode flag
 */
typedef enum vmf_dce_encrypt_mode
{
	VMF_ENCRYPT_MODE_EBC = 0,
	VMF_ENCRYPT_MODE_CBC = 1,
	VMF_ENCRYPT_MODE_CFB = 2,
	VMF_ENCRYPT_MODE_OFB = 3,
	VMF_ENCRYPT_MODE_CTR = 4
} VMF_DCE_ENCRYPT_MODE;

/*
 * DataCrypto encrypto type flag
 */
typedef enum vmf_dce_encrypt_type
{
	VMF_ENCRYPT_TYPE_AES = 0,
	VMF_ENCRYPT_TYPE_TDES = 1,
	VMF_ENCRYPT_TYPE_DES = 2
} VMF_DCE_ENCRYPT_TYPE;

/*
 * DataCrypto hash type flag
 */
typedef enum vmf_dce_hash_type
{
	VMF_DCE_HASH_TYPE_SHA_1 = 0,
	VMF_DCE_HASH_TYPE_SHA_256 = 2,
	VMF_DCE_HASH_TYPE_SHA_224 = 3,
	VMF_DCE_HASH_TYPE_SHA_512 = 4,
	VMF_DCE_HASH_TYPE_SHA_384 = 5
} VMF_DCE_HASH_TYPE;

/*
 * DataCrypto hash mode flag
 */
typedef enum vmf_dce_hash_mode
{
	VMF_DCE_HASH_MODE_HASHING_ONLY = 0,
	VMF_DCE_HASH_MODE_HMAC = 1
} VMF_DCE_HASH_MODE;

/*
 * DataCrypto hash stat flag
 */
typedef enum vmf_dce_hash_stat
{
	VMF_DCE_HASH_STAT_BEGIN = 0,
	VMF_DCE_HASH_STAT_END = 1,
	VMF_DCE_HASH_STAT_MID = 2
} VMF_DCE_HASH_STAT;

/*
 * A data structure for cipher 
 */
typedef struct vmf_dce_cipher_t
{
	//!encrypto type flag: NCRYPT_AES, ENCRYPT_TDES, ENCRYPT_DES
	VMF_DCE_ENCRYPT_TYPE eCryptoType;	

	//!encrypto mode flag: EBC_MODE, CBC_MODE, CFB_MODE, OFB_MODE, CTR_MODE
	VMF_DCE_ENCRYPT_MODE eCryptoMode;	

	//! A data for key size
	unsigned int dwKeySize;
			
}VMF_DCE_CIPHER_T;

/*
 * A data structure for hash 
 */
typedef struct vmf_dce_hash_t
{
	//!hash type flag: HASH_SHA_1, HASH_SHA_256, HASH_SHA_224, HASH_SHA_512, HASH_SHA_384
	VMF_DCE_HASH_TYPE eHashType;		

	//!hash mode flag:	HASHING_ONLY, HMAC
	VMF_DCE_HASH_MODE eHashMode;	

	//!hash stat flag:  HASH_BEGIN, HASH_END, HASH_MIDDLE			
	VMF_DCE_HASH_STAT eHashStat;	

	//! A data for Hash size
	unsigned int dwHashSize;	
}VMF_DCE_HASH_T;

/*
 * A data structure for DataCrypto 
 */
typedef struct vmf_dce_state_t
{
	//!operation flag: OP_ENCRYPTION, OP_DECRYPTION, OP_HASH
	VMF_DCE_OP_FLAG eOpMode;		

	//! A data for text size
	unsigned int dwTextSize;					

	//! A data for DCE data info (VMF_DCE_CIPHER_T or VMF_DCE_HASH_T)
	void* ptDceInfo;			
} VMF_DCE_STATE_T;

typedef struct vmf_dce_handle_t VMF_DCE_HANDLE_T;

typedef struct vmf_dce_initopt_t {
	//! A data for key virtual buffer, 128byte alignment	
	unsigned char *pbyKeyVirtBuff;	

	//! A data for initialization vector buffer, 16byte	 alignment	
	unsigned char *pbyInitVectorVirtBuff;	

	//! A data for input virtual buffer, 128byte alignment	 
	unsigned char *pbyInputVirtBuff;		

	//! A data for outoput virtual buffer, 128byte alignment		
	unsigned char *pbyOutputVirtBuff;	
} VMF_DCE_INITOPT_T;

/**
 * @brief Function to initialize datacrypto handle.
 *
 * @param[in] ptInitOpt Initial options for DCE handle.
 * @return The handle of DCE handle.
 */
VMF_DCE_HANDLE_T* VMF_DCE_Init(const VMF_DCE_INITOPT_T* ptInitOpt);

/**
 * @brief Function to release datacrypto handle.
 *
 * @param[in] ptHandle The datacrypto handle.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_DCE_Release(VMF_DCE_HANDLE_T* ptHandle);

/**
 * @brief Function to process datacrypto one frame.
 *
 * @param[in] pHandle The handle of datacrypto.
 * @param[in] pState The state of datacrypto.
 * @return Success: 0 Fail: negative integer.
 */
int VMF_DCE_ProcessOneFrame(VMF_DCE_HANDLE_T* ptHandle, VMF_DCE_STATE_T *pState);

#ifdef __cplusplus
}
#endif

#endif 