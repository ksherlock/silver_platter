#ifndef __HTTP_H__
#define __HTTP_H__


// http status codes
// from perl HTTP::Status

#define RC_CONTINUE                         100
#define RC_SWITCHING_PROTOCOLS              101
#define RC_PROCESSING                       102
#define RC_EARLY_HINTS                      103

#define RC_OK                               200
#define RC_CREATED                          201
#define RC_ACCEPTED                         202
#define RC_NON_AUTHORITATIVE_INFORMATION    203
#define RC_NO_CONTENT                       204
#define RC_RESET_CONTENT                    205
#define RC_PARTIAL_CONTENT                  206
#define RC_MULTI_STATUS                     207
#define RC_ALREADY_REPORTED                 208
#define RC_IM_USED                          226

#define RC_MULTIPLE_CHOICES                 300
#define RC_MOVED_PERMANENTLY                301
#define RC_FOUND                            302
#define RC_SEE_OTHER                        303
#define RC_NOT_MODIFIED                     304
#define RC_USE_PROXY                        305
#define RC_TEMPORARY_REDIRECT               307
#define RC_PERMANENT_REDIRECT               308

#define RC_BAD_REQUEST                      400
#define RC_UNAUTHORIZED                     401
#define RC_PAYMENT_REQUIRED                 402
#define RC_FORBIDDEN                        403
#define RC_NOT_FOUND                        404
#define RC_METHOD_NOT_ALLOWED               405
#define RC_NOT_ACCEPTABLE                   406
#define RC_PROXY_AUTHENTICATION_REQUIRED    407
#define RC_REQUEST_TIMEOUT                  408
#define RC_CONFLICT                         409
#define RC_GONE                             410
#define RC_LENGTH_REQUIRED                  411
#define RC_PRECONDITION_FAILED              412
#define RC_REQUEST_ENTITY_TOO_LARGE         413
#define RC_REQUEST_URI_TOO_LARGE            414
#define RC_UNSUPPORTED_MEDIA_TYPE           415
#define RC_REQUEST_RANGE_NOT_SATISFIABLE    416
#define RC_EXPECTATION_FAILED               417
#define RC_MISDIRECTED REQUEST              421
#define RC_UNPROCESSABLE_ENTITY             422
#define RC_LOCKED                           423
#define RC_FAILED_DEPENDENCY                424
#define RC_UPGRADE_REQUIRED                 426
#define RC_PRECONDITION_REQUIRED            428
#define RC_TOO_MANY_REQUESTS                429
#define RC_REQUEST_HEADER_FIELDS_TOO_LARGE  431
#define RC_UNAVAILABLE_FOR_LEGAL_REASONS    451

#define RC_INTERNAL_SERVER_ERROR            500
#define RC_NOT_IMPLEMENTED                  501
#define RC_BAD_GATEWAY                      502
#define RC_SERVICE_UNAVAILABLE              503
#define RC_GATEWAY_TIMEOUT                  504
#define RC_HTTP_VERSION_NOT_SUPPORTED       505
#define RC_VARIANT_ALSO_NEGOTIATES          506
#define RC_INSUFFICIENT_STORAGE             507
#define RC_LOOP_DETECTED                    508
#define RC_NOT_EXTENDED                     510
#define RC_NETWORK_AUTHENTICATION_REQUIRED  511

/* unofficial */
#define RC_I_AM_A_TEAPOT                    418
#define RC_UNORDERED_COLLECTION             425
#define RC_RETRY_WITH                       449
#define RC_BANDWIDTH_LIMIT_EXCEEDED         509

#define is_info(e)          (e >= 100 && e < 200)
#define is_success(e)       (e >= 200 && e < 300)
#define is_redirect(e)      (e >= 300 && e < 400)
#define is_client_error(e)  (e >= 400 && e < 500)
#define is_server_error(e)  (e >= 500 && e < 600)

#define is_error(e)         (e >= 400 && e < 600)


#endif
