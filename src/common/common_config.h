#pragma once

#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#define BOOST_MPL_LIMIT_VECTOR_SIZE 50
#define FUSION_MAX_VECTOR_SIZE BOOST_MPL_LIMIT_VECTOR_SIZE
#define BOOST_MPL_LIMIT_STRING_SIZE BOOST_MPL_LIMIT_VECTOR_SIZE
#if defined(OS_POSIX) || defined( __GNUC__ )
#define BOOST_PP_VARIADICS
//#define __GCCXML__
#endif

#define INVALID_DESCRIPTOR -1
#define ERROR_RESULT_VALUE -1
