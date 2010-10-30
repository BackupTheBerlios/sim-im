#ifndef CORE_API_H
#define CORE_API_H


#ifndef CORE_EXPORT
#   ifdef CORE_EXPORTS
#       define CORE_EXPORT Q_DECL_EXPORT
#   else // CORE_EXPORTS
#       define CORE_EXPORT Q_DECL_IMPORT
#   endif // CORE_EXPORTS
#endif // CORE_EXPORT


#endif // CORE_API_H
