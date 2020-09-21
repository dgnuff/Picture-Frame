#pragma once

// This assumes that T has a copy constructor and operator=
// Don't make T too large, since they do get copied around a bit.  Note the std::copy towards the bottom of nextItem().
template<typename T>
class shuffle
{
public:
    shuffle(T error) :
    position_(0),
    visible_(0),
    error_(error)
    {
    }

    virtual ~shuffle()
    {
    }

    void add_item(T payload)
    {
        items_.push_back(item(payload, position_++));
    }

    // This directly sets the size of the output window to a value between 1 and 3/4 the set size.
    // Note that setting the factor to 1 means that the sequence repeats without alteration.
    void set_volatility(uint32 factor)
    {
        if (factor > (uint32) items_.size() * 3 / 4)
        {
            factor = (uint32) items_.size() * 3 / 4;
        }
        if (factor < 1)
        {
            factor = 1;
        }
        visible_ = factor;
    }

    // Set the volatility to a number between 0.0 and 0.75 - this sets the size of the output window,
    // the larger is it, the more volatile the shuffle will be
    void set_volatility(float factor)
    {
        if (factor < 0.0f)
        {
            factor = 0.0f;
        }
        set_volatility((uint32) ((float) items_.size() * factor));
    }

    // Shuffle the current set.  This is typically executed once per session immediatly after placing all elements in the set.
    void shuffle_payload()
    {
        // Classic Fisher-Yates shuffle.
        for (int32 i = (int32) items_.size() - 1; i > 0; i--)
        {
            int32 const j = rnd32(i + 1);
            std::swap(items_[i]._payload, items_[j]._payload);
        }
    }

    void clear()
    {
        position_ = 0;
        visible_ = 0;
        items_.clear();
    }

    // Fetch the next item
    T next_item()
    {
        // return error value if nothing in the set
        if (items_.size() == 0)
        {
            return error_;
        }
        int32 resultIndex = 0;
        // Figure how long the oldest item has been in the window
        int32 const timeInWindow = position_ - items_[0].position_ - (int32) items_.size() + visible_;

        // Only select randomly if the oldest item has been there less than twice the window duration.
        if (timeInWindow < (int32) visible_ * 2)
        {
            // In which case, pick one at random.
            resultIndex = rnd32(visible_);
        }

        auto first = items_.begin() + (resultIndex + 1);
        auto last = items_.end();
        auto target = items_.begin() + resultIndex;
        item result = items_[resultIndex];
        std::copy(first, last, target);
        items_[items_.size() - 1]._payload = result._payload;
        items_[items_.size() - 1].position_ = position_++;
        return result._payload;
    }

private:
    class item
    {
    public:
        T _payload;
        int32 position_;

        item(T payload, int32 position) :
        _payload(payload),
        position_(position)
        {
        }

        virtual ~item() {}
    };

    std::vector<item> items_;
    uint32 position_;
    uint32 visible_;
    T error_;
};
