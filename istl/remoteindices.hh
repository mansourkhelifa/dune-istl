// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_REMOTEINDICES_HH
#define DUNE_REMOTEINDICES_HH

#include <dune/istl/indexset.hh>
#include <dune/common/poolallocator.hh>
#include <dune/common/sllist.hh>
#include <map>
#include <utility>
#include <iostream>
#include <dune/istl/mpitraits.hh>
#include "mpi.h"

namespace Dune {

  /**
   * @brief An index present on the local process with an additional attribute.
   */
  template<typename T>
  class ParallelLocalIndex
  {
    // friend declaration needed for MPITraits
    friend class MPITraits<ParallelLocalIndex<T> >;

  public:
    /**
     * @brief The type of the attributes.
     * Normally this will be an enumeration like
     * <pre>
     * enum Attributes{owner, border, overlap};
     * </pre>
     */
    typedef T AttributeType;
    /**
     * @brief Constructor.
     *
     * @param localIndex The local index.
     * @param attribute The attribute of the index.
     * @param isPublic True if the index might also be
     * known to other processes.
     */
    ParallelLocalIndex(const AttributeType& attribute, bool isPublic);

    /**
     * @brief Constructor.
     *
     * The local index will be initialized to 0.
     * @param attribute The attribute of the index.
     * @param isPublic True if the index might also be
     * known to other processes.
     */
    ParallelLocalIndex(uint32_t localIndex, const AttributeType& attribute, bool isPublic);
    /**
     * @brief Parameterless constructor.
     *
     * Needed for use in FixedArray.
     */
    ParallelLocalIndex();

    /**
     * @brief Constructor.
     * @param globalIndex The global index.
     * @param attribute The attribute of the index.
     * @param local The local index.
     * @param isPublic True if the index might also be
     * known to other processes.
     *
       ParallelLocalIndex(const AttributeType& attribute, uint32_t local,
       bool isPublic);
     */
    /**
     * @brief Get the attribute of the index.
     * @return The associated attribute.
     */
    inline const AttributeType attribute() const;

    /**
     * @brief Set the attribute of the index.
     * @param attribute The associated attribute.
     */
    inline void setAttribute(const AttributeType& attribute);

    /**
     * @brief get the local index.
     * @return The local index.
     */
    inline uint32_t local() const;

    /**
     * @brief Convert to the local index represented by an int.
     */
    inline operator uint32_t() const;

    /**
     * @brief Assign a new local index.
     *
     * @param index The new local index.
     */
    inline ParallelLocalIndex<AttributeType>& operator=(uint32_t index);

    /**
     * @brief Check whether the index might also be known other processes.
     * @return True if the index might be known to other processors.
     */
    inline bool isPublic() const;

    /**
     * @brief Get the state.
     * @return The state.
     */
    inline const LocalIndexState state() const;

    /**
     * @brief Set the state.
     * @param state The state to set.
     */
    inline void setState(const LocalIndexState& state);

  private:
    /** @brief The local index. */
    uint32_t localIndex_;

    /** @brief An attribute for the index. */
    char attribute_;

    /** @brief True if the index is also known to other processors. */
    char public_;

    /**
     * @brief The state of the index.
     *
     * Has to be one of LocalIndexState!
     * @see LocalIndexState.
     */
    char state_;

  };

  template<typename T>
  class MPITraits<ParallelLocalIndex<T> >
  {
  public:
    static MPI_Datatype getType();
  private:
    static MPI_Datatype type;

  };

  template<typename TG, typename TA>
  class MPITraits<IndexPair<TG,ParallelLocalIndex<TA> > >
  {
  public:
    inline static MPI_Datatype getType();
  private:
    static MPI_Datatype type;
  };


  template<typename T1, typename T2>
  class RemoteIndices;

  template<typename T1, typename T2>
  class RemoteIndex;

  template<typename T1, typename T2>
  std::ostream& operator<<(std::ostream& os, const RemoteIndex<T1,T2>& index);

  /**
   * @brief Information about an index residing on another processor.
   */
  template<typename T1, typename T2>
  class RemoteIndex
  {
    friend class RemoteIndices<T1,T2>;
    friend std::ostream& operator<<<>(std::ostream& os, const RemoteIndex<T1,T2>& index);

  public:
    /**
     * @brief the type of the global index.
     * This type has to provide at least a operator&lt; for sorting.
     */
    typedef T1 GlobalIndexType;
    /**
     * @brief The type of the attributes.
     * Normally this will be an enumeration like
     * <pre>
     * enum Attributes{owner, border, overlap}
     * </pre>
     */
    typedef T2 AttributeType;

    /**
     * @brief The type of the index pair.
     */
    typedef IndexPair<GlobalIndexType,ParallelLocalIndex<AttributeType> >
    PairType;

    /**
     * @brief Get the attribute of the index on the remote process.
     * @return The remote attribute.
     */
    const AttributeType attribute() const;

    /**
     * @brief Get the corresponding local index pair.
     * @return The corresponding local index pair.
     */

    const PairType& localIndexPair() const;

    /**
     * @brief Parameterless Constructor.
     */
    RemoteIndex();

  private:
    /**
     * @brief Constructor.
     * Private as it should only be called from within Indexset.
     * @param attribute The attribute of the index on the remote processor.
     * @param local The corresponding local index.
     */
    RemoteIndex(const T2& attribute,
                const PairType* local);


    /** @brief The corresponding local index for this process. */
    const PairType* localIndex_;

    /** @brief The attribute of the index on the other process. */
    char attribute_;
  };

  template<class TG, class TA>
  std::ostream& operator<<(std::ostream& os, const RemoteIndices<TG,TA>& indices);

  /**
   * @brief The indices present on remote processes.
   */
  template<class TG,class TA>
  class RemoteIndices
  {
    friend std::ostream& operator<<<>(std::ostream& os, const RemoteIndices<TG,TA>& indices);

  public:
    /**
     * @brief The type of the global index.
     */
    typedef TG GlobalIndexType;

    /**
     * @brief The type of the attribute.
     */
    typedef TA AttributeType;

    /**
     * @brief The type of the local index.
     */
    typedef ParallelLocalIndex<AttributeType> LocalIndexType;

    /**
     * @brief Type of the index set we use.
     */
    typedef IndexSet<int,LocalIndexType > IndexSetType;

    /**
     * @brief Type of the remote indices we manage.
     */
    typedef RemoteIndex<GlobalIndexType,AttributeType> RemoteIndexType;

    /**
     * @brief Constructor.
     * @param comm The communicator to use.
     * @param source The indexset which represents the global to
     * local mapping at the source of the communication
     * @param to The indexset to which the communication which represents the global to
     * local mapping at the destination of the communication.
     * May be the same as the source indexset.
     */
    inline RemoteIndices(const IndexSetType& source, const IndexSetType& destination,
                         const MPI_Comm& comm);

    /**
     * @brief Rebuilds the set of remote indices.
     *
     * This has to be called whenever the underlying index sets
     * change.
     *
     * If the template parameter ignorePublic is true all indices will be treated
     * as public.
     */
    template<bool ignorePublic>
    void rebuild();

    /**
     * @brief Checks whether the remote indices are synced with
     * the indexsets.
     *
     * If they are not synced the remote indices need to be rebuild.
     * @return True if they are synced.
     */
    inline bool isSynced();

  private:
    /** @brief Index set used at the source of the communication. */
    const IndexSetType& source_;

    /** @brief Index set used at the destination of the communication. */
    const IndexSetType& dest_;

    /** @brief The communicator to use.*/
    const MPI_Comm& comm_;

    /** @brief The communicator tag to use. */
    const static int commTag_=333;

    /**
     * @brief The sequence number of the source index set when the remote indices
     * where build.
     */
    int sourceSeqNo_;

    /**
     * @brief The sequence number of the destination index set when the remote indices
     * where build.
     */
    int destSeqNo_;

    /** @brief The index pair type. */
    typedef IndexPair<GlobalIndexType, ParallelLocalIndex<AttributeType> >
    PairType;
    /** @brief The type of the remote index list. */
    typedef SLList<RemoteIndex<GlobalIndexType,AttributeType> >
    RemoteIndexList;

    /**
     * @brief The remote indices.
     *
     * The key is the process id and the values are the pair of remote
     * index lists, the first for receiving, the second for sending.
     */
    std::map<int, std::pair<RemoteIndexList*,RemoteIndexList*> > remoteIndices_;

    /* @brief The index pairs for local copying if from and to differ. */
    SLList<std::pair<int,int> > copyLocal_;

    /**
     * @brief Build the local mapping.
     *
     * If the template parameter ignorePublic is true all indices will be treated
     * as public.
     */
    template<bool ignorePublic>
    inline void buildLocal();

    /**
     * @brief Build the remote mapping.
     *
     * If the template parameter ignorePublic is true all indices will be treated
     * as public.
     */
    template<bool ignorePublic>
    inline void buildRemote();

    /**
     * @brief Pack the indices to send if source_ and dest_ are the same.
     *
     * If the template parameter ignorePublic is true all indices will be treated
     * as public.
     * @param myPairs Array to store references to the public indices in.
     * @param p_out The output buffer to pack the entries to.
     * @param type The mpi datatype for the pairs.
     * @param bufferSize The size of the output buffer p_out.
     * @param position The position to start packing.
     */
    template<bool ignorePublic>
    inline void packEntries(PairType** myPairs, const IndexSetType& indexSet,
                            char* p_out, MPI_Datatype type, int bufferSize,
                            int* position);

    /**
     * @brief unpacks the received indices and builds the remote index list.
     *
     * @param remote The list to add the indices to.
     * @param remoteEntries The number of remote entries to unpack.
     * @param local The local indices to check wether we know the remote
     *              indices.
     * @param localEntries The number of local indices.
     * @param type The mpi data type for unpacking.
     * @param p_in The input buffer to unpack from.
     * @param postion The position in the buffer to start unpacking from.
     * @param bufferSize The size of the input buffer.
     */
    inline void unpackIndices(RemoteIndexList& remote, int remoteEntries,
                              PairType** local, int localEntries, char* p_in,
                              MPI_Datatype type, int* positon, int bufferSize);

  };

  template<class T>
  ParallelLocalIndex<T>::ParallelLocalIndex(const T& attribute, bool isPublic)
    : localIndex_(0), attribute_(static_cast<char>(attribute)),
      public_(static_cast<char>(isPublic)), state_(static_cast<char>(VALID))
  {}


  template<class T>
  ParallelLocalIndex<T>::ParallelLocalIndex(uint32_t local, const T& attribute, bool isPublic)
    : localIndex_(local), attribute_(static_cast<char>(attribute)),
      public_(static_cast<char>(isPublic)), state_(static_cast<char>(VALID))
  {}

  template<class T>
  ParallelLocalIndex<T>::ParallelLocalIndex()
    : localIndex_(0), attribute_(), public_(static_cast<char>(false)),
      state_(static_cast<char>(VALID))
  {}

  /*
      template<class T>
      ParallelLocalIndex<T>::ParallelLocalIndex(const T& attribute, uint32_t local,
      bool isPublic)
      : localIndex_(local), attribute_(attribute), public_(isPublic),
      state_(VALID)
      {}
   */

  template<class T>
  inline const T ParallelLocalIndex<T>::attribute() const
  {
    return T(attribute_);
  }

  template<class T>
  inline void
  ParallelLocalIndex<T>::setAttribute(const AttributeType& attribute)
  {
    attribute_ = attribute;
  }

  template<class T>
  inline uint32_t ParallelLocalIndex<T>::local() const
  {
    return localIndex_;
  }

  template<class T>
  inline ParallelLocalIndex<T>::operator uint32_t() const
  {
    return localIndex_;
  }

  template<class T>
  inline ParallelLocalIndex<T>&
  ParallelLocalIndex<T>::operator=(uint32_t index)
  {
    localIndex_=index;
    return *this;
  }

  template<class T>
  inline bool ParallelLocalIndex<T>::isPublic() const
  {
    return static_cast<bool>(public_);
  }

  template<class T>
  inline const LocalIndexState ParallelLocalIndex<T>::state() const
  {
    return LocalIndexState(state_);
  }

  template<class T>
  inline void ParallelLocalIndex<T>::setState(const LocalIndexState& state)
  {
    state_=static_cast<char>(state);
  }

  template<typename T>
  MPI_Datatype MPITraits<ParallelLocalIndex<T> >::getType()
  {

    if(type==MPI_DATATYPE_NULL) {
      int length[3];
      MPI_Aint disp[3];
      MPI_Datatype types[3] = {MPI_LB, MPITraits<char>::getType(), MPI_UB};
      ParallelLocalIndex<T> rep[2];
      length[0]=length[1]=length[2]=1;
      MPI_Address(rep, disp); // lower bound of the datatype
      MPI_Address(&(rep[0].attribute_), disp+1);
      MPI_Address(rep+1, disp+2); // upper bound od the datatype
      for(int i=2; i >= 0; --i)
        disp[i] -= disp[0];
      MPI_Type_struct(3, length, disp, types, &type);
      MPI_Type_commit(&type);
    }
    return type;
  }

  template<typename T>
  MPI_Datatype MPITraits<ParallelLocalIndex<T> >::type = MPI_DATATYPE_NULL;

  template<typename TG, typename TA>
  MPI_Datatype MPITraits<IndexPair<TG,ParallelLocalIndex<TA> > >::getType()
  {
    if(type==MPI_DATATYPE_NULL) {
      int length[4];
      MPI_Aint disp[4];
      MPI_Datatype types[4] = {MPI_LB, MPITraits<TG>::getType(),
                               MPITraits<ParallelLocalIndex<TA> >::getType(), MPI_UB};
      IndexPair<TG,ParallelLocalIndex<TA> > rep[2];
      length[0]=length[1]=length[2]=length[3]=1;
      MPI_Address(rep, disp); // lower bound of the datatype
      MPI_Address(&(rep[0].global_), disp+1);
      MPI_Address(&(rep[0].local_), disp+2);
      MPI_Address(rep+1, disp+3); // upper bound of the datatype
      for(int i=3; i >= 0; --i)
        disp[i] -= disp[0];
      MPI_Type_struct(4, length, disp, types, &type);
      MPI_Type_commit(&type);
    }
    return type;
  }

  template<typename TG, typename TA>
  MPI_Datatype MPITraits<IndexPair<TG,ParallelLocalIndex<TA> > >::type=MPI_DATATYPE_NULL;

  template<typename T1, typename T2>
  RemoteIndex<T1,T2>::RemoteIndex(const T2& attribute, const PairType* local)
    : localIndex_(local), attribute_(attribute)
  {}


  template<typename T1, typename T2>
  RemoteIndex<T1,T2>::RemoteIndex()
    : localIndex_(0), attribute_()
  {}

  template<typename T1, typename T2>
  inline const T2 RemoteIndex<T1,T2>::attribute() const
  {
    return T2(attribute_);
  }

  template<typename T1, typename T2>
  inline const IndexPair<T1,ParallelLocalIndex<T2> >& RemoteIndex<T1,T2>::localIndexPair() const
  {
    return *localIndex_;
  }

  template<class TG, class TA>
  inline RemoteIndices<TG,TA>::RemoteIndices(const IndexSetType& source,
                                             const IndexSetType& destination,
                                             const MPI_Comm& comm)
    : source_(source), dest_(destination), comm_(comm),
      sourceSeqNo_(-1), destSeqNo_(-1)
  {}

  template<class TG, class TA>
  template<bool ignorePublic>
  inline void RemoteIndices<TG,TA>::packEntries(IndexPair<GlobalIndexType,LocalIndexType>** pairs,
                                                const IndexSetType& indexSet,
                                                char* p_out, MPI_Datatype type,
                                                int bufferSize,
                                                int *position)
  {
    // fill with own indices
    typedef typename IndexSetType::const_iterator const_iterator;
    typedef IndexPair<GlobalIndexType,LocalIndexType> PairType;
    const const_iterator end = indexSet.end();

    //Now pack the source indices
    int i=0;
    for(const_iterator index = indexSet.begin(); index != end; ++index)
      if(ignorePublic || index->local().isPublic()) {

        MPI_Pack(const_cast<PairType*>(&(*index)), 1,
                 type,
                 p_out, bufferSize, position, comm_);
        pairs[i++] = const_cast<PairType*>(&(*index));

      }
  }

  template<class TG, class TA>
  template<bool ignorePublic>
  inline void RemoteIndices<TG,TA>::buildLocal()
  {
    //typedef typename IndexSetType::iterator const_iterator;
    typedef Dune::ConstArrayListIterator<Dune::IndexPair<int, Dune::ParallelLocalIndex<TA> >, 100,
        std::allocator<Dune::IndexPair<int, Dune::ParallelLocalIndex<TA> > > > const_iterator;

    copyLocal_.clear();

    const_iterator sourceIndex = source_.begin(), destIndex = dest_.begin();
    const_iterator sourceEnd = source_.end(), destEnd = dest_.end();
    int i=0;

    while(sourceIndex != sourceEnd && destIndex != destEnd) {
      if(destIndex->global() == sourceIndex->global() &&
         ( ignorePublic || destIndex->local().isPublic() && sourceIndex->local().isPublic()))
      {
        copyLocal_.push_back(std::make_pair(sourceIndex->local(), destIndex->local()));
        ++sourceIndex;
        ++destIndex;
        ++i;
      }
      else if(destIndex->global() < sourceIndex->global())
        ++destIndex;
      else
        ++sourceIndex;
    }
  }

  template<class TG, class TA>
  template<bool ignorePublic>
  inline void RemoteIndices<TG,TA>::buildRemote()
  {
    remoteIndices_.clear();

    // Processor configuration
    int rank, procs;
    MPI_Comm_rank(comm_, &rank);
    MPI_Comm_size(comm_, &procs);
    if(procs==1)
      // nothing to do in sequential mode
      return;

    // number of local indices to publish
    // The indices of the destination will be send.
    int sourcePublish, destPublish;

    // Do we need to send two index sets?
    bool sendTwo = (&source_ != &dest_);

    sourcePublish = (ignorePublic) ? source_.size() : source_.noPublic();

    if(sendTwo)
      destPublish = (ignorePublic) ? dest_.size() : dest_.noPublic();
    else
      // we only need to send one set of indices
      destPublish = 0;

    int maxPublish, publish=sourcePublish+destPublish;

    // Calucate maximum number of indices send
    MPI_Allreduce(&publish, &maxPublish, 1, MPI_INT, MPI_MAX, comm_);

    // allocate buffers
    typedef IndexPair<GlobalIndexType,LocalIndexType> PairType;

    PairType** destPairs;
    PairType** sourcePairs = new PairType*[sourcePublish];

    if(sendTwo)
      destPairs = new PairType*[destPublish];
    else
      destPairs=sourcePairs;

    char** buffer = new char*[2];
    int bufferSize;
    int position=0;
    int intSize;

    // calculate buffer size
    MPI_Datatype type = MPITraits<PairType>::getType();

    MPI_Pack_size(maxPublish, type, comm_,
                  &bufferSize);
    MPI_Pack_size(1, MPI_INT, comm_,
                  &intSize);
    // Our message will contain the following:
    // a bool wether two index sets where sent
    // the size of the source and the dest indexset,
    // then the source and destination indices
    bufferSize += 3 * intSize;

    buffer[0] = new char[bufferSize];
    buffer[1] = new char[bufferSize];

    // send mesages in ring
    for(int proc=1; proc<procs; proc++) {
      // pointers to the current input and output buffers
      char* p_out = buffer[1-(proc%2)];
      char* p_in = buffer[proc%2];

      if(proc==1) {

        MPI_Pack(&sendTwo, 1, MPI_INT, p_out, bufferSize, &position,
                 comm_);

        // The number of indices we send for each index set
        MPI_Pack(&sourcePublish, 1, MPI_INT, p_out, bufferSize, &position,
                 comm_);
        MPI_Pack(&destPublish, 1, MPI_INT, p_out, bufferSize, &position,
                 comm_);

        // Now pack the source indices and setup the destination pairs
        packEntries<ignorePublic>(sourcePairs, source_, p_out, type,
                                  bufferSize, &position);
        // If necessary send the dest indices and setup the source pairs
        if(sendTwo)
          packEntries<ignorePublic>(destPairs, dest_, p_out, type,
                                    bufferSize, &position);

        //std::cout<<rank<<": Publishing "<<sourcePublish<<" source and "
        // <<destPublish<<" destination indices!"<<std::endl<<std::flush;

      }

      MPI_Status status;

      if(rank%2==0) {
        MPI_Ssend(p_out, position, MPI_PACKED, (rank+1)%procs,
                  commTag_, comm_);
        MPI_Recv(p_in, bufferSize, MPI_PACKED, (rank+procs-1)%procs,
                 commTag_, comm_, &status);
      }else{
        MPI_Recv(p_in, bufferSize, MPI_PACKED, (rank+procs-1)%procs,
                 commTag_, comm_, &status);
        MPI_Ssend(p_out, position, MPI_PACKED, (rank+1)%procs,
                  commTag_, comm_);
      }

      // unpack the number of indices we received
      int noReceive, noSend, twoIndexSets;
      position=0;
      // Did we receive two index sets?
      MPI_Unpack(p_in, bufferSize, &position, &twoIndexSets, 1, MPI_INT, comm_);
      // The number of source indices received
      MPI_Unpack(p_in, bufferSize, &position, &noReceive, 1, MPI_INT, comm_);
      // The number of destination indices received
      MPI_Unpack(p_in, bufferSize, &position, &noSend, 1, MPI_INT, comm_);

      // The process these indices are from
      int remoteProc = (rank+procs-proc)%procs;

      //typedef SLList<RemoteIndex<GlobalIndexType,AttributeType> >
      //	RemoteIndexList;


      RemoteIndexList* receive= new RemoteIndexList();
      RemoteIndexList* send;

      unpackIndices(*receive, noReceive, destPairs, destPublish,
                    p_in, type, &position, bufferSize);


      if(twoIndexSets || sendTwo) {
        // Indices may differ in each direction
        send = new RemoteIndexList();
        unpackIndices(*send, noSend, sourcePairs, sourcePublish,
                      p_in, type, &position, bufferSize);
      }else
        send = receive;

      if(send->empty() && receive->empty()) {
        if(send==receive) {
          delete send;
        }else{
          delete send;
          delete receive;
        }
      }else{
        remoteIndices_.insert(std::make_pair(remoteProc,
                                             std::make_pair(send,receive)));
      }
    }
    // delete allocated memory
    if(destPairs!=sourcePairs)
      delete[] destPairs;

    delete[] sourcePairs;
    delete[] buffer[0];
    delete[] buffer[1];
  }

  template<class TG, class TA>
  inline void RemoteIndices<TG,TA>::unpackIndices(RemoteIndexList& remote,
                                                  int remoteEntries,
                                                  PairType** local,
                                                  int localEntries,
                                                  char* p_in,
                                                  MPI_Datatype type,
                                                  int* position,
                                                  int bufferSize)
  {
    if(remoteEntries==0)
      return;

    PairType index;
    MPI_Unpack(p_in, bufferSize, position, &index, 1,
               type, comm_);

    int n_in=0, localIndex=0;

    //Check if we know the global index
    while(localIndex<localEntries) {
      if(local[localIndex]->global()==index.global()) {
        remote.push_back(RemoteIndex<TG,TA>(index.local().attribute(),
                                            local[localIndex]));
        localIndex++;
        // unpack next remote index
        if((++n_in) < remoteEntries) {
          MPI_Unpack(p_in, bufferSize, position, &index, 1,
                     type, comm_);
        }else{
          // No more received indices
          break;
        }
      }else if (local[localIndex]->global()<index.global()) {
        ++localIndex;
      }else{
        // We do not know the index, unpack next
        if((++n_in) < remoteEntries) {
          MPI_Unpack(p_in, bufferSize, position, &index, 1,
                     type, comm_);
        }else
          // No more received indices
          break;
      }
    }
  }


  template<class TG, class TA>
  template<bool ignorePublic>
  inline void RemoteIndices<TG,TA>::rebuild()
  {
    //if(&source_ != &dest_)
    //buildLocal<ignorePublic>();
    buildRemote<ignorePublic>();
  }

  template<class TG, class TA>
  inline bool RemoteIndices<TG,TA>::isSynced()
  {
    return sourceSeqNo_==source_.seqNo() && destSeqNo_ ==dest_.seqNo();
  }

  template<typename TG, typename TA>
  inline std::ostream& operator<<(std::ostream& os, const RemoteIndex<TG,TA>& index)
  {
    os<<"[global="<<index.localIndexPair().global()<<",attribute="<<index.attribute()<<"]";
    return os;
  }

  template<class TG, class TA>
  inline std::ostream& operator<<(std::ostream& os, const RemoteIndices<TG,TA>& indices)
  {
    int rank;
    MPI_Comm_rank(indices.comm_, &rank);

    if(!indices.copyLocal_.empty()) {
      typedef typename SLList<std::pair<int,int> >::const_iterator const_iterator;

      const const_iterator end=indices.copyLocal_.end();
      const_iterator pair=indices.copyLocal_.begin();
      if(pair!=end) {

        os<<rank<<": Copying local: ";

        for(; pair !=end; ++pair)
          os<<pair->first<<"->"<<pair->second<<", ";

        os<<std::endl<<std::flush;
      }

    }
    typedef SLList<RemoteIndex<TG,TA> > RList;
    typedef typename std::map<int,std::pair<RList*,RList*> >::const_iterator const_iterator;

    const const_iterator rend = indices.remoteIndices_.end();

    for(const_iterator rindex = indices.remoteIndices_.begin(); rindex!=rend; ++rindex) {
      os<<rank<<": Prozess "<<rindex->first<<":";

      if(!rindex->second.first->empty()) {
        os<<" send:";

        const typename RList::const_iterator send= rindex->second.first->end();

        for(typename RList::const_iterator index = rindex->second.first->begin();
            index != send; ++index)
          os<<*index<<" ";
        os<<std::endl;
      }
      if(!rindex->second.second->empty()) {
        os<<rank<<": Prozess "<<rindex->first<<": "<<"receive: ";

        const typename RList::const_iterator rend= rindex->second.second->end();

        for(typename RList::const_iterator index = rindex->second.second->begin();
            index != rend; ++index)
          os<<*index<<" ";
      }
      os<<std::endl<<std::flush;
    }
    return os;
  }
}

#endif