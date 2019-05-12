#ifndef _NET_CALLBACKS_H_
#define _NET_CALLBACKS_H_

#include <functional>

namespace maou
{
typedef std::function<void(int)> EventsCallback;
}

#endif
