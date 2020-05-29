#include "Link.h"

namespace nodephysics {

/**************************************************************************/
/*********************************LINK*************************************/
/**************************************************************************/

template<class T>
Link<T>::Link(const BaseLink::InitLink& init)
    : BaseLink(init)
{
}

template<class T>
Link<T>::~Link()
{
}

template<class T>
void Link<T>::setLinkedDest(T* linkedDest)
{
    BaseLink::setLinkedDest(linkedDest);
}

template<class T>
T* Link<T>::getLinkedDest()
{
    return dynamic_cast<T*>(m_linkedDest->getBase());
}

template<class T>
T* Link<T>::operator->() { return getLinkedDest(); }

template<class T>
T* Link<T>::operator*() { return getLinkedDest(); }

template<class T>
void Link<T>::operator=(T* o) { this->setLinkedDest(o); }

template<class T>
void Link<T>::operator=(typename T::SPtr o) { this->setLinkedDest(o.get()); }

} // namespace nodephysics
