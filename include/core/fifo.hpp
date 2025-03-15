#include <array>

template<typename T, std::size_t Size>
class StaticFifo {
private:
    std::array<T, Size> buffer {};
    std::size_t head = 0;
    std::size_t tail = 0;
    std::size_t count = 0;
    static constexpr std::size_t BatchSize = 8;

public:
    bool pushBatch( const std::array<T, BatchSize>& values ) {
        if( count + BatchSize > Size )
            return false;

        for( std::size_t i = 0; i < BatchSize; ++i ) {
            buffer[tail] = values[i];
            tail = ( tail + 1 ) % Size;
        }
        count += BatchSize;
        return true;
    }

    std::array<T, BatchSize> popBatch() {
        std::array<T, BatchSize> result {};
        if( count >= BatchSize ) {
            for( std::size_t i = 0; i < BatchSize; ++i ) {
                result[i] = buffer[head];
                head = ( head + 1 ) % Size;
            }
            count -= BatchSize;
        }
        return result;
    }

    bool empty() const {
        return count == 0;
    }

    bool full() const {
        return count >= Size;
    }

    std::size_t size() const {
        return count;
    }

    std::size_t capacity() const {
        return Size;
    }

    void clear() {
        head = 0;
        tail = 0;
        count = 0;
    }
};
