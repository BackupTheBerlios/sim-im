#ifndef ICQ_DEFINES_H
#define ICQ_DEFINES_H


#ifndef ICQ_EXPORT
#   ifdef ICQ_EXPORTS
#       define ICQ_EXPORT Q_DECL_EXPORT
#   else
#       define ICQ_EXPORT Q_DECL_IMPORT
#   endif
#endif


#endif // ICQ_DEFINES_H
