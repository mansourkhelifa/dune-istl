// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ISTL_MULTITYPEBLOCKVECTOR_HH
#define DUNE_ISTL_MULTITYPEBLOCKVECTOR_HH

#include <cmath>
#include <iostream>
#include <tuple>

#include <dune/common/dotproduct.hh>
#include <dune/common/ftraits.hh>
#include <dune/common/hybridutilities.hh>

#include "istlexception.hh"

// forward declaration
namespace Dune {
  template < typename... Args >
  class MultiTypeBlockVector;
}

#include "gsetc.hh"

namespace Dune {

  /**
      @addtogroup ISTL_SPMV
      @{
   */




  /** \brief Calculate the 2-norm

     Each element of the vector has to provide the method "two_norm2()"
     in order to calculate the whole vector's 2-norm.
   */
  template<int count, typename T>
  class MultiTypeBlockVector_Norm {
  public:
    typedef typename T::field_type field_type;
    typedef typename FieldTraits<field_type>::real_type real_type;

    /**
     * sum up all elements' 2-norms
     */
    static real_type result (const T& a) {             //result = sum of all elements' 2-norms
      return std::get<count-1>(a).two_norm2() + MultiTypeBlockVector_Norm<count-1,T>::result(a);
    }
  };

  template<typename T>                                    //recursion end: no more vector elements to add...
  class MultiTypeBlockVector_Norm<0,T> {
  public:
    typedef typename T::field_type field_type;
    typedef typename FieldTraits<field_type>::real_type real_type;
    static real_type result (const T&) {return 0.0;}
  };

  /** \brief Calculate the \infty-norm

     Each element of the vector has to provide the method "infinity_norm()"
     in order to calculate the whole vector's norm.
   */
  template<int count, typename T,
           bool hasNaN = has_nan<typename T::field_type>::value>
  class MultiTypeBlockVector_InfinityNorm {
  public:
    typedef typename T::field_type field_type;
    typedef typename FieldTraits<field_type>::real_type real_type;

    /**
     * Take the maximum over all elements' norms
     */
    static real_type result (const T& a)
    {
      std::pair<real_type, real_type> const ret = resultHelper(a);
      return ret.first * (ret.second / ret.second);
    }

    // Computes the maximum while keeping track of NaN values
    static std::pair<real_type, real_type> resultHelper(const T& a) {
      using std::max;
      auto const rest =
          MultiTypeBlockVector_InfinityNorm<count - 1, T>::resultHelper(a);
      real_type const norm = std::get<count - 1>(a).infinity_norm();
      return {max(norm, rest.first), norm + rest.second};
    }
  };

  template <int count, typename T>
  class MultiTypeBlockVector_InfinityNorm<count, T, false> {
  public:
    typedef typename T::field_type field_type;
    typedef typename FieldTraits<field_type>::real_type real_type;

    /**
     * Take the maximum over all elements' norms
     */
    static real_type result (const T& a)
    {
      using std::max;
      return max(std::get<count-1>(a).infinity_norm(), MultiTypeBlockVector_InfinityNorm<count-1,T>::result(a));
    }
  };

  template<typename T, bool hasNaN>                       //recursion end
  class MultiTypeBlockVector_InfinityNorm<0,T, hasNaN> {
  public:
    typedef typename T::field_type field_type;
    typedef typename FieldTraits<field_type>::real_type real_type;
    static real_type result (const T&)
    {
      return 0.0;
    }

    static std::pair<real_type,real_type> resultHelper (const T&)
    {
      return {0.0, 1.0};
    }
  };

  /** @addtogroup DenseMatVec
      @{
   */
  template <typename Arg0, typename... Args>
  struct FieldTraits< MultiTypeBlockVector<Arg0, Args...> >
  {
    typedef typename FieldTraits<Arg0>::field_type field_type;
    typedef typename FieldTraits<Arg0>::real_type real_type;
  };
  /**
      @}
   */

  /**
      @brief A Vector class to support different block types

      This vector class combines elements of different types known at compile-time.
   */
  template < typename... Args >
  class MultiTypeBlockVector
  : public std::tuple<Args...>
  {
    /** \brief Helper type */
    typedef std::tuple<Args...> tupleType;
  public:

    /**
     * own class' type
     */
    typedef MultiTypeBlockVector<Args...> type;

    /** \brief The type used for scalars
     *
     * The current code hardwires it to 'double', which is far from nice.
     * On the other hand, it is not clear what the correct type is.  If the MultiTypeBlockVector class
     * is instantiated with several vectors of different field_types, what should the resulting
     * field_type be?
     */
    typedef double field_type;

    /** \brief Return the number of vector entries */
    static constexpr std::size_t size()
    {
      return sizeof...(Args);
    }

    /**
     * number of elements
     */
    int count()
    {
      return sizeof...(Args);
    }

    /** \brief Random-access operator
     *
     * This method mimicks the behavior of normal vector access with square brackets like, e.g., v[5] = 1.
     * The problem is that the return type is different for each value of the argument in the brackets.
     * Therefore we implement a trick using std::integral_constant.  To access the first entry of
     * a MultiTypeBlockVector named v write
     * \code
     *  MultiTypeBlockVector<A,B,C,D> v;
     *  std::integral_constant<std::size_t,0> _0;
     *  v[_0] = ...
     * \endcode
     * The name '_0' used here as a static replacement of the integer number zero is arbitrary.
     * Any other variable name can be used.  If you don't like the separate variable, you can writee
     * \code
     *  MultiTypeBlockVector<A,B,C,D> v;
     *  v[std::integral_constant<std::size_t,0>()] = ...
     * \endcode
     */
    template< std::size_t index >
    typename std::tuple_element<index,tupleType>::type&
    operator[] ( const std::integral_constant< std::size_t, index > indexVariable )
    {
      DUNE_UNUSED_PARAMETER(indexVariable);
      return std::get<index>(*this);
    }

    /** \brief Const random-access operator
     *
     * This is the const version of the random-access operator.  See the non-const version for a full
     * explanation of how to use it.
     */
    template< std::size_t index >
    const typename std::tuple_element<index,tupleType>::type&
    operator[] ( const std::integral_constant< std::size_t, index > indexVariable ) const
    {
      DUNE_UNUSED_PARAMETER(indexVariable);
      return std::get<index>(*this);
    }

    /** \brief Assignment operator
     */
    template<typename T>
    void operator= (const T& newval) {
      Dune::Hybrid::forEach(*this, [&](auto&& entry) {
        entry = newval;
      });
    }

    /**
     * operator for MultiTypeBlockVector += MultiTypeBlockVector operations
     */
    void operator+= (const type& newv) {
      using namespace Dune::Hybrid;
      forEach(integralRange(Hybrid::size(*this)), [&](auto&& i) {
        (*this)[i] += newv[i];
      });
    }

    /**
     * operator for MultiTypeBlockVector -= MultiTypeBlockVector operations
     */
    void operator-= (const type& newv) {
      using namespace Dune::Hybrid;
      forEach(integralRange(Hybrid::size(*this)), [&](auto&& i) {
        (*this)[i] -= newv[i];
      });
    }

    // Once we have the IsNumber traits class the following
    // three implementations could be replaced by:
    //
    //    template<class T,
    //      std::enable_if_t< IsNumber<T>::value, int> = 0>
    //    void operator*= (const T& w) {
    //      Dune::Hybrid::forEach(*this, [&](auto&& entry) {
    //        entry *= w;
    //      });
    //    }

    void operator*= (const int& w) {
      Dune::Hybrid::forEach(*this, [&](auto&& entry) {
        entry *= w;
      });
    }

    void operator*= (const float& w) {
      Dune::Hybrid::forEach(*this, [&](auto&& entry) {
        entry *= w;
      });
    }

    void operator*= (const double& w) {
      Dune::Hybrid::forEach(*this, [&](auto&& entry) {
        entry *= w;
      });
    }

    field_type operator* (const type& newv) const {
      using namespace Dune::Hybrid;
      field_type result = 0;
      forEach(integralRange(Hybrid::size(*this)), [&](auto&& i) {
        result += (*this)[i]*newv[i];
      });
      return result;
    }

    field_type dot (const type& newv) const {
      using namespace Dune::Hybrid;
      field_type result = 0;
      forEach(integralRange(Hybrid::size(*this)), [&](auto&& i) {
        result += (*this)[i].dot(newv[i]);
      });
      return result;
    }

    /** \brief Compute the squared Euclidean norm
     */
    typename FieldTraits<field_type>::real_type two_norm2() const {return MultiTypeBlockVector_Norm<sizeof...(Args),type>::result(*this);}

    /** \brief Compute the Euclidean norm
     */
    typename FieldTraits<field_type>::real_type two_norm() const {return sqrt(this->two_norm2());}

    /** \brief Compute the maximum norm
     */
    typename FieldTraits<field_type>::real_type infinity_norm() const
    {
      return MultiTypeBlockVector_InfinityNorm<sizeof...(Args),type>::result(*this);
    }

    /** \brief Axpy operation on this vector (*this += a * y)
     *
     * \tparam Ta Type of the scalar 'a'
     */
    template<typename Ta>
    void axpy (const Ta& a, const type& y) {
      using namespace Dune::Hybrid;
      forEach(integralRange(Hybrid::size(*this)), [&](auto&& i) {
        (*this)[i].axpy(a, y[i]);
      });
    }

  };



  /** \brief Send MultiTypeBlockVector to an outstream
   */
  template <typename... Args>
  std::ostream& operator<< (std::ostream& s, const MultiTypeBlockVector<Args...>& v) {
    using namespace Dune::Hybrid;
    forEach(integralRange(size(v)), [&](auto&& i) {
      s << "\t(" << i << "):\n" << v[i] << "\n";
    });
    return s;
  }



} // end namespace

#endif
