
template<int t_length>
class AllNearestNeighborsGnp {
 public:
  typedef StaticVector<t_length> Point;
  typedef StaticHrectBound<t_length> Bound;

 private:
  struct QResult {
    double distance_sq;
    index_t index;

    void Init() {
      distance_sq = DBL_MAX;
      index = -1;
    }

    void Update(const QPoint &q, const RPoint &r, index_t r_index) {
      double trial_distance_sq = la::DistanceSqEuclidean(q, r);

      if (unlikely(trial_distance_sq < distance_sq)) {
        distance_sq = trial_distance_sq;
        index = r_index;
      }
    }
  };

  struct QResultStat {
    double worst_distance_sq;

    void Init() {
      worst_distance_sq = DBL_MAX;
    }

    void StartUpdate() {
      worst_distance_sq = -DBL_MAX;
    }

    void Update(const Result& subresult) {
      double distance_sq = subresult.distance_sq;
      if (unlikely(worst_distance_sq < distance_sq)) {
        worst_distance_sq = distance_sq;
      }
    }

    void Update(const QResultStat& substat, index_t substat_n_points) {
      if (substat.worst_distance_sq > worst_distance_sq) {
        worst_distance_sq = substat.worst_distance_sq;
      }
    }

    void FinishUpdate(index_t n_points) {
      /* no finish op necessary */
    }
  };

  struct Algorithm {
    void Init(datanode *module, GlobalStat *gstat) {}

    bool TryPrune(
        const Bound &rbound, const EmptyStat &rstat,
        const QPoint &qpoint, QResult *qresult,
        GlobalStat *gstat) const {
      bool can_prune =
          bound::MinDistanceSqToInstance(qbound, qpoint) > qresult.distance_sq;
      return can_prune;
    }

    bool TryPrune(
        const Bound &r_bound, const EmptyStat &r_stat,
        const Bound &q_bound, const EmptyStat &q_stat,
        QResultStat *q_result_stat, EmptyMassResult *q_delta,
        GlobalStat *g_stat) const {
      bool can_prune = q_bound.MinDistanceSqToBound(r_bound)
          > q_result_stat->worst_distance_sq;
      return can_prune;
    }

    double Prioritize(
        const Bound &r_bound, const EmptyStat &r_stat,
        const Bound &q_bound, const EmptyStat &q_stat,
        const QResultStat &q_result_stat,
        const GlobalStat &gstat) const {
      return qbound.MidDistanceSqToBound(r_bound);
    }
  };

 public:
  /** The full GNP for this problem. */
  typedef GnpQueryReference<
      Algorithm,
      Point, Bound,
      EmptyStat,
      EmptyStat, QResultStat, EmptyMassResult, Result,
      EmptyGlobalStat,
      AllNearestNeighborGNP
  > GNP;
  typedef GnpQueryReferenceRunner<GNP> Runner;

 public:
  void Run(datanode *params) {
    Runner runner;
    runner.Run(params);
  }
};
