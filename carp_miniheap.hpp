
#ifndef CARP_MINIHEAP_INCLUDED
#define CARP_MINIHEAP_INCLUDED

template<typename T>
class CarpMiniHeap
{
public:
    CarpMiniHeap() { }
    ~CarpMiniHeap() { free(m_elements); }

public:
    unsigned int GetSize() const { return m_size; }
    T* GetElement(unsigned int index) { return m_elements[index]; }
    bool IsEmpty() const { return 0 == m_size; }
    T* GetTop() { return m_size > 0 ? *m_elements : nullptr; }
	
    bool Push(T* element)
    {
        if (m_size == 0xFFFFFFFF || !Reserve(m_size + 1))
            return false;
    	
        ShitUp(m_size++, element);
        return true;
    }

    T* Pop()
    {
        if (m_size <= 0) return nullptr;
    	
        T* element = *m_elements;
        ShiftDown(0u, m_elements[--m_size]);
        element->SetHeapIndex(-1);
        return element;
    }
	
    bool Adjust(T* element)
    {
        if (-1 == element->GetHeapIndex()) return Push(element);
        
        unsigned int parent = (element->GetHeapIndex() - 1) / 2;
        /* The position of e has changed; we shift it up or down
        * as needed.  We can't need to do both. */
        if (element->GetHeapIndex() > 0 && *m_elements[parent] > *element)
            ShiftUpUnconditional(element->GetHeapIndex(), element);
        else
            ShiftDown(element->GetHeapIndex(), element);

        return true;
    }

    bool Erase(T* element)
    {
        if (-1 == element->GetHeapIndex()) return false;
    	
        T* last = m_elements[--m_size];
        unsigned int parent = (element->GetHeapIndex() - 1) / 2;
        /* we replace e with the last element in the heap.  We might need to
        shift it upward if it is less than its parent, or downward if it is
        greater than one or both its children. Since the children are known
        to be less than the parent, it can't need to shift both up and
        down. */
        if (element->GetHeapIndex() > 0 && *m_elements[parent] > *last)
            ShiftUpUnconditional(element->GetHeapIndex(), last);
        else
            ShiftDown(element->GetHeapIndex(), last);
        element->SetHeapIndex(-1);
        return true;
    }

private:
    bool Reserve(unsigned int size)
    {
        if (m_capacity >= size)
            return true;

        unsigned int capacity = m_capacity * 2;
        if (capacity < size) capacity = size;

        T** elements = static_cast<T**>(std::realloc(m_elements, capacity * sizeof * elements));
        if (!elements) return false;

        m_elements = elements;
        m_capacity = capacity;

        return true;
    }

    void ShitUp(unsigned int hole_index, T* element)
    {
        unsigned int parent = (hole_index - 1) / 2;
        while (hole_index > 0 && *m_elements[parent] > *element)
        {
            (m_elements[hole_index] = m_elements[parent])->SetHeapIndex(hole_index);
            hole_index = parent;
            parent = (hole_index - 1) / 2;
        }
        (m_elements[hole_index] = element)->SetHeapIndex(hole_index);
    }

    void ShiftDown(unsigned int hole_index, T* element)
    {
        unsigned int min_child = 2 * (hole_index + 1);
        while (min_child <= m_size)
        {
            min_child -= min_child == m_size || *m_elements[min_child] > *m_elements[min_child - 1];
            if (!(*element > *m_elements[min_child])) break;

            (m_elements[hole_index] = m_elements[min_child])->SetHeapIndex(hole_index);
            hole_index = min_child;
            min_child = 2 * (hole_index + 1);
        }
        (m_elements[hole_index] = element)->SetHeapIndex(hole_index);
    }

    void ShiftUpUnconditional(unsigned int hole_index, T* element)
    {
        unsigned int parent = (hole_index - 1) / 2;
        do {
            (m_elements[hole_index] = m_elements[parent])->SetHeapIndex(hole_index);
            hole_index = parent;
            parent = (hole_index - 1) / 2;
        } while (hole_index && *m_elements[parent] > *element);
        (m_elements[hole_index] = element)->SetHeapIndex(hole_index);
    }

private:
    T** m_elements = nullptr;
    unsigned m_size = 0;
    unsigned m_capacity = 0;
};

#endif