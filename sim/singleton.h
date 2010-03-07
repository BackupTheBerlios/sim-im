#ifndef GALAXYSINGLETON_H
#define GALAXYSINGLETON_H

#include <cassert>

namespace SIM
{
template <typename Type> class EXPORT Singleton
{
	static Type* m_instance;
        // no need to implement them!
	Singleton(const Singleton<Type>&);
	Singleton<Type>& operator=(const Singleton<Type>& t);
public:
	Singleton()
	{
		assert(!m_instance);
		m_instance = static_cast<Type*>(this);
	}
	virtual ~Singleton()
	{
		m_instance = 0;
	};
	static Type* instance()
	{
        //fprintf(stderr, "instance: %p\n", m_instance);
		assert(m_instance);
		return m_instance;
	}

};
// This should be here, otherwise compiler will complain about
// unresolved externals
#if !defined(WIN32) && !defined(QT_VISIBILITY_AVAILABLE)
	template <typename Type> Type* Singleton<Type>::m_instance = 0;
#endif
}
#endif
