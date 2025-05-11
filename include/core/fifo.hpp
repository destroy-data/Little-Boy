#include <array>

template<typename T, std::size_t N>
class StaticFifo {
private:
    std::array<T, N> buffer {};
    std::size_t head = 0;
    std::size_t tail = 0;
    std::size_t count = 0;
    static constexpr std::size_t BatchN = 8;

public:
    bool push( const T& value ) {
        if( count >= N )
            return false;
        buffer[tail] = value;
        tail = ( tail + 1 ) % N;
        ++count;
        return true;
    }

    T pop() {
        T result {};
        if( count > 0 ) {
            result = buffer[head];
            head = ( head + 1 ) % N;
            --count;
        }
        return result;
    }

    bool empty() const {
        return count == 0;
    }

    bool full() const {
        return count >= N;
    }

    std::size_t size() const {
        return count;
    }

    std::size_t capacity() const {
        return N;
    }

    void clear() {
        head = 0;
        tail = 0;
        count = 0;
    }
};
