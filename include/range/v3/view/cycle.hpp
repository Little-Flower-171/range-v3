/// \file cycle.hpp
// Range v3 library
//
//  Copyright Eric Niebler 2013-2015
//  Copyright Gonzalo Brito Gadeschi 2015
//  Copyright Casey Carter 2015
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/ericniebler/range-v3
//

#ifndef RANGES_V3_VIEW_CYCLE_HPP
#define RANGES_V3_VIEW_CYCLE_HPP

#include <type_traits>
#include <meta/meta.hpp>
#include <range/v3/range_fwd.hpp>
#include <range/v3/size.hpp>
#include <range/v3/distance.hpp>
#include <range/v3/begin_end.hpp>
#include <range/v3/range_traits.hpp>
#include <range/v3/range_concepts.hpp>
#include <range/v3/view_facade.hpp>
#include <range/v3/view/all.hpp>
#include <range/v3/view/view.hpp>
#include <range/v3/utility/iterator.hpp>
#include <range/v3/utility/static_const.hpp>

namespace ranges
{
    inline namespace v3
    {
        /// \addtogroup group-views
        ///@{
        template<typename Rng>
        struct cycled_view
          : view_facade<cycled_view<Rng>, infinite>
        {
        private:
            CONCEPT_ASSERT(ForwardRange<Rng>());
            friend range_access;
            Rng rng_;

            template<bool IsConst>
            struct cursor
            {
            private:
                template<typename T>
                using constify_if = meta::apply<meta::add_const_if_c<IsConst>, T>;
                using cycled_view_t = constify_if<cycled_view>;
                using difference_type_ = range_difference_t<Rng>;

            private:
                cycled_view_t *rng_;
                range_iterator_t<constify_if<Rng>> it_;

            public:
                cursor() = default;
                explicit cursor(cycled_view_t &rng)
                  : rng_(&rng), it_(ranges::begin(rng.rng_))
                {}
                constexpr bool done() const
                {
                    return false;
                }
                auto current() const
                RANGES_DECLTYPE_AUTO_RETURN_NOEXCEPT
                (
                    *it_
                )
                bool equal(cursor const &pos) const
                {
                    RANGES_ASSERT(rng_ == pos.rng_);
                    return it_ == pos.it_;
                }
                void next()
                {
                    auto const end = ranges::end(rng_->rng_);
                    RANGES_ASSERT(it_ != end);
                    if(++it_ == end)
                        it_ = ranges::begin(rng_->rng_);
                }
                CONCEPT_REQUIRES(BidirectionalRange<Rng>{})
                void prev()
                {
                    if(it_ == ranges::begin(rng_->rng_))
                        it_ = ranges::end(rng_->rng_);
                    --it_;
                }
                CONCEPT_REQUIRES(RandomAccessRange<Rng>())
                void advance(difference_type_ n)
                {
                    auto const begin = ranges::begin(rng_->rng_);
                    auto const end = ranges::end(rng_->rng_);
                    auto const d = end - begin;
                    auto off = ((it_ - begin) + n) % d;
                    it_ = begin + (off < 0 ? off + d : off);
                }
                CONCEPT_REQUIRES(RandomAccessRange<Rng>())
                difference_type_ distance_to(cursor const &that) const
                {
                    RANGES_ASSERT(that.rng_ == rng_);
                    return that.it_ - it_;
                }
            };

            cursor<false> begin_cursor()
            {
                return cursor<false>{*this};
            }
            CONCEPT_REQUIRES(Range<Rng const>())
            cursor<true> begin_cursor() const
            {
                return cursor<true>{*this};
            }

        public:
            cycled_view() = default;
            explicit cycled_view(Rng rng)
              : rng_(std::move(rng))
            {
                RANGES_ASSERT(ranges::distance(rng) != 0);
            }
        };

        namespace view
        {
            struct cycle_fn
            {
            private:
                friend view_access;
                template<class T>
                using Concept = meta::and_<ForwardRange<T>, BoundedRange<T>>;

            public:
                template<typename Rng, CONCEPT_REQUIRES_(Concept<Rng>())>
                cycled_view<all_t<Rng>> operator()(Rng &&rng) const
                {
                    return cycled_view<all_t<Rng>>{all(std::forward<Rng>(rng))};
                }

#ifndef RANGES_DOXYGEN_INVOKED
                template<typename Rng, CONCEPT_REQUIRES_(!Concept<Rng>())>
                void operator()(Rng &&) const
                {
                    CONCEPT_ASSERT_MSG(ForwardRange<Rng>(),
                                       "The object on which view::cycle operates must be a "
                                       "model of the ForwardRange concept.");
                    CONCEPT_ASSERT_MSG(BoundedRange<Rng>(),
                                       "To cycle a range object, its end iterator must be a "
                                       "model of the ForwardIterator concept.");
                }
#endif
            };

            /// \relates cycle_fn
            /// \ingroup group-views
            namespace
            {
                constexpr auto &&cycle = static_const<view<cycle_fn>>::value;
            }

       } // namespace view
       /// @}
    } // namespace v3
} // namespace ranges

#endif
