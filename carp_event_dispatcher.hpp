#ifndef CARP_EVENT_DISPATCHER_INCLUDED
#define CARP_EVENT_DISPATCHER_INCLUDED

#include <list>
#include <functional>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <memory>

template <typename E>
class CarpEventTypeValue
{
public:
    static size_t Value() { return typeid(E).hash_code(); }
};

template <typename LISTENER_BASE, typename EVENT_BASE, template <typename EVENT> class EVENT_TYPE = CarpEventTypeValue>
class CarpEventDispatcher
{
public:
    virtual ~CarpEventDispatcher() {}

public:
    // 分发事件
    template<typename E>
    void DispatchEventImpl(const E& event)
    {
        // 锁定
        Lock();

        // 遍历，逐个调用
        auto it_listener = m_listener.find(EVENT_TYPE<E>::Value());
        if (it_listener != m_listener.end())
        {
            for (auto it = it_listener->second.begin(); it != it_listener->second.end();)
            {
                auto listener = it->first.lock();

                // 如果指针还存在，那么就执行回调函数，如果没有就自动移除
                if (!listener)
                {
                    it = it_listener->second.erase(it);
                    continue;
                }

                it->second(event);

                ++it;
            }
        }

        // 解锁
        Unlock();
    }

public:
    // 监听事件
    template<typename E, typename T>
    void AddEventListener(const std::weak_ptr<T>& weak_listener)
    {
        auto listener = weak_listener.lock();
        if (!listener) return;

        if (IsLock())
        {
            m_lock_list.push_back([this, weak_listener]() { AddEventListener<E, T>(weak_listener); });
            return;
        }

        // 获取类型信息
        static_assert(std::is_base_of<EVENT_BASE, E>::value, "any class must extends EVENT_BASE");
        static_assert(std::is_final<E>::value, "any class must define final");
        const auto event_type = EVENT_TYPE<E>::Value();

        // 转为基本类型
        auto base_listener = std::static_pointer_cast<LISTENER_BASE>(listener);

        // 添加到映射表
        m_listener[event_type][base_listener] = [weak_listener](const EVENT_BASE& event)
        {
            auto listener = weak_listener.lock();
            if (!listener) return;

            const E* e = dynamic_cast<const E*>(&event);
            if (e == nullptr) return;
            listener->OnEvent(*e);
        };

        // 添加反向映射信息
        m_object[base_listener].insert(event_type);
    }

public:
    // 移除事件
    template <typename E>
    void RemoveEventListener(const std::weak_ptr<LISTENER_BASE>& weak_listener)
    {
        if (IsLock())
        {
            m_lock_list.push_back([this, weak_listener]() { RemoveEventListener<E>(weak_listener); });
            return;
        }

        // 查反向映射
        auto it_object = m_object.find(weak_listener);
        if (it_object == m_object.end()) return;

        // 获取事件类型
        const auto event_type = EVENT_TYPE<E>::Value();

        // 获取类型
        auto it_event_type = it_object->second.find(event_type);
        if (it_event_type == it_object->second.end()) return;

        auto it_listener = m_listener.find(event_type);
        if (it_listener == m_listener.end()) return;

        it_listener->second.erase(weak_listener);
        if (it_listener->second.empty()) m_listener.erase(it_listener);

        it_object->second.erase(it_event_type);
        if (it_object->second.empty()) m_object.erase(it_object);
    }

    // 移除事件
    void RemoveEventListener(const std::weak_ptr<LISTENER_BASE>& weak_listener)
    {
        if (IsLock())
        {
            m_lock_list.push_back([this, weak_listener]() { RemoveEventListener(weak_listener); });
            return;
        }

        auto it_object = m_object.find(weak_listener);
        if (it_object == m_object.end()) return;

        for (auto& event_type : it_object->second)
        {
            auto it_listener = m_listener.find(event_type);
            if (it_listener == m_listener.end()) continue;

            it_listener->second.erase(weak_listener);
            if (it_listener->second.empty()) m_listener.erase(it_listener);
        }

        m_object.erase(it_object);
    }

    void ClearEventListener()
    {
        if (IsLock())
        {
            m_lock_list.push_back([this]() { ClearEventListener(); });
            return;
        }

        m_object.clear();
        m_listener.clear();
    }

private:
    // 锁定m_listener以及子内容
    void Lock()
    {
        ++m_lock;
    }

    // 解锁
    void Unlock()
    {
        --m_lock;
        if (m_lock > 0) return;
        if (m_lock_list.empty()) return;

        for (auto& callback : m_lock_list)
            callback();
        m_lock_list.clear();
    }

    // 判断是否处于锁定状态
    bool IsLock() const { return m_lock > 0; }

private:
    std::map<std::weak_ptr<LISTENER_BASE>, std::unordered_set<size_t>, std::owner_less<std::weak_ptr<LISTENER_BASE>>> m_object;
    std::unordered_map<size_t, std::map<std::weak_ptr<LISTENER_BASE>, std::function<void(const EVENT_BASE&)>, std::owner_less<std::weak_ptr<LISTENER_BASE>>>> m_listener;

private:
    int m_lock = 0;
    std::list<std::function<void()>> m_lock_list;
};


#endif