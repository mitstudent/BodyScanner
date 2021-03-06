/*
 * Software License Agreement (BSD License)
 *
 *  Point Cloud Library (PCL) - www.pointclouds.org
 *  Copyright (c) 2009-2010, Willow Garage, Inc.
 *
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Willow Garage, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: sac_model_cylinder.h 2326 2011-08-31 07:48:25Z rusu $
 *
 */
 
#ifndef PCL_SAMPLE_CONSENSUS_MODEL_CYLINDER_H_
#define PCL_SAMPLE_CONSENSUS_MODEL_CYLINDER_H_

#include <pcl/sample_consensus/sac_model.h>
#include <pcl/sample_consensus/model_types.h>
#include <boost/thread/mutex.hpp>
#include <pcl/common/common.h>

namespace pcl
{
  /** \brief @b SampleConsensusModelCylinder defines a model for 3D cylinder segmentation.
    * The model coefficients are defined as:
    * <ul>
    * <li><b>point_on_axis.x</b>  : the X coordinate of a point located on the cylinder axis
    * <li><b>point_on_axis.y</b>  : the Y coordinate of a point located on the cylinder axis
    * <li><b>point_on_axis.z</b>  : the Z coordinate of a point located on the cylinder axis
    * <li><b>axis_direction.x</b> : the X coordinate of the cylinder's axis direction
    * <li><b>axis_direction.y</b> : the Y coordinate of the cylinder's axis direction
    * <li><b>axis_direction.z</b> : the Z coordinate of the cylinder's axis direction
    * <li><b>radius</b>           : the cylinder's radius
    * </ul>
    * \author Radu Bogdan Rusu
    * \ingroup sample_consensus
    */
  template <typename PointT, typename PointNT>
  class SampleConsensusModelCylinder : public SampleConsensusModel<PointT>, public SampleConsensusModelFromNormals<PointT, PointNT>
  {
    using SampleConsensusModel<PointT>::input_;
    using SampleConsensusModel<PointT>::indices_;
    using SampleConsensusModel<PointT>::radius_min_;
    using SampleConsensusModel<PointT>::radius_max_;
    using SampleConsensusModelFromNormals<PointT, PointNT>::normals_;
    using SampleConsensusModelFromNormals<PointT, PointNT>::normal_distance_weight_;

    public:
      typedef typename SampleConsensusModel<PointT>::PointCloud PointCloud;
      typedef typename SampleConsensusModel<PointT>::PointCloudPtr PointCloudPtr;
      typedef typename SampleConsensusModel<PointT>::PointCloudConstPtr PointCloudConstPtr;

      typedef boost::shared_ptr<SampleConsensusModelCylinder> Ptr;

      /** \brief Constructor for base SampleConsensusModelCylinder.
        * \param[in] cloud the input point cloud dataset
        */
      SampleConsensusModelCylinder (const PointCloudConstPtr &cloud) : SampleConsensusModel<PointT> (cloud), eps_angle_ (0) 
      {
        axis_.setZero ();
      }

      /** \brief Constructor for base SampleConsensusModelCylinder.
        * \param[in] cloud the input point cloud dataset
        * \param[in] indices a vector of point indices to be used from \a cloud
        */
      SampleConsensusModelCylinder (const PointCloudConstPtr &cloud, const std::vector<int> &indices) : SampleConsensusModel<PointT> (cloud, indices), eps_angle_ (0)
      {
        axis_.setZero ();
      }

      /** \brief Set the angle epsilon (delta) threshold.
        * \param[in] ea the maximum allowed difference between the cyilinder axis and the given axis.
        */
      inline void 
      setEpsAngle (const double ea) { eps_angle_ = ea; }

      /** \brief Get the angle epsilon (delta) threshold. */
      inline double 
      getEpsAngle () { return (eps_angle_); }

      /** \brief Set the axis along which we need to search for a cylinder direction.
        * \param[in] ax the axis along which we need to search for a cylinder direction
        */
      inline void 
      setAxis (const Eigen::Vector3f &ax) { axis_ = ax; }

      /** \brief Get the axis along which we need to search for a cylinder direction. */
      inline Eigen::Vector3f 
      getAxis ()  { return (axis_); }

      /** \brief Get 2 random points with their normals as data samples and return them as point indices.
        * \param[out] iterations the internal number of iterations used by SAC methods
        * \param[out] samples the resultant model samples
        * \note assumes unique points!
        */
      void 
      getSamples (int &iterations, std::vector<int> &samples);

      /** \brief Check whether the given index samples can form a valid cylinder model, compute the model coefficients
        * from these samples and store them in model_coefficients. The cylinder coefficients are: point_on_axis,
        * axis_direction, cylinder_radius_R
        * \param[in] samples the point indices found as possible good candidates for creating a valid model
        * \param[out] model_coefficients the resultant model coefficients
        */
      bool 
      computeModelCoefficients (const std::vector<int> &samples, 
                                Eigen::VectorXf &model_coefficients);

      /** \brief Compute all distances from the cloud data to a given cylinder model.
        * \param[in] model_coefficients the coefficients of a cylinder model that we need to compute distances to
        * \param[out] distances the resultant estimated distances
        */
      void 
      getDistancesToModel (const Eigen::VectorXf &model_coefficients, 
                           std::vector<double> &distances);

      /** \brief Select all the points which respect the given model coefficients as inliers.
        * \param[in] model_coefficients the coefficients of a cylinder model that we need to compute distances to
        * \param[in] threshold a maximum admissible distance threshold for determining the inliers from the outliers
        * \param[out] inliers the resultant model inliers
        */
      void 
      selectWithinDistance (const Eigen::VectorXf &model_coefficients, 
                            const double threshold, 
                            std::vector<int> &inliers);

      /** \brief Count all the points which respect the given model coefficients as inliers. 
        * 
        * \param[in] model_coefficients the coefficients of a model that we need to compute distances to
        * \param[in] threshold maximum admissible distance threshold for determining the inliers from the outliers
        * \return the resultant number of inliers
        */
      virtual int
      countWithinDistance (const Eigen::VectorXf &model_coefficients, 
                           const double threshold);

      /** \brief Recompute the cylinder coefficients using the given inlier set and return them to the user.
        * @note: these are the coefficients of the cylinder model after refinement (eg. after SVD)
        * \param[in] inliers the data inliers found as supporting the model
        * \param[in] model_coefficients the initial guess for the optimization
        * \param[out] optimized_coefficients the resultant recomputed coefficients after non-linear optimization
        */
      void 
      optimizeModelCoefficients (const std::vector<int> &inliers, 
                                 const Eigen::VectorXf &model_coefficients, 
                                 Eigen::VectorXf &optimized_coefficients);


      /** \brief Create a new point cloud with inliers projected onto the cylinder model.
        * \param[in] inliers the data inliers that we want to project on the cylinder model
        * \param[in] model_coefficients the coefficients of a cylinder model
        * \param[out] projected_points the resultant projected points
        * \param[in] copy_data_fields set to true if we need to copy the other data fields
        */
      void 
      projectPoints (const std::vector<int> &inliers, 
                     const Eigen::VectorXf &model_coefficients, 
                     PointCloud &projected_points, 
                     bool copy_data_fields = true);

      /** \brief Verify whether a subset of indices verifies the given cylinder model coefficients.
        * \param[in] indices the data indices that need to be tested against the cylinder model
        * \param[in] model_coefficients the cylinder model coefficients
        * \param[in] threshold a maximum admissible distance threshold for determining the inliers from the outliers
        */
      bool 
      doSamplesVerifyModel (const std::set<int> &indices, 
                            const Eigen::VectorXf &model_coefficients, 
                            const double threshold);

      /** \brief Return an unique id for this model (SACMODEL_CYLINDER). */
      inline pcl::SacModel 
      getModelType () const { return (SACMODEL_CYLINDER); }

    protected:
      /** \brief Get the distance from a point to a line (represented by a point and a direction)
        * \param[in] pt a point
        * \param[in] model_coefficients the line coefficients (a point on the line, line direction)
        */
      double 
      pointToLineDistance (const Eigen::Vector4f &pt, const Eigen::VectorXf &model_coefficients);

      /** \brief Project a point onto a line given by a point and a direction vector
        * \param[in] pt the input point to project
        * \param[in] line_pt the point on the line (make sure that line_pt[3] = 0 as there are no internal checks!)
        * \param[in] line_dir the direction of the line (make sure that line_dir[3] = 0 as there are no internal checks!)
        * \param[out] pt_proj the resultant projected point
        */
      inline void
      projectPointToLine (const Eigen::Vector4f &pt, 
                          const Eigen::Vector4f &line_pt, 
                          const Eigen::Vector4f &line_dir,
                          Eigen::Vector4f &pt_proj)
      {
        double k = (pt.dot (line_dir) - line_pt.dot (line_dir)) / line_dir.dot (line_dir);
        // Calculate the projection of the point on the line
        pt_proj = line_pt + k * line_dir;
      }

      /** \brief Project a point onto a cylinder given by its model coefficients (point_on_axis, axis_direction,
        * cylinder_radius_R)
        * \param[in] pt the input point to project
        * \param[in] model_coefficients the coefficients of the cylinder (point_on_axis, axis_direction, cylinder_radius_R)
        * \param[out] pt_proj the resultant projected point
        */
      void 
      projectPointToCylinder (const Eigen::Vector4f &pt, 
                              const Eigen::VectorXf &model_coefficients, 
                              Eigen::Vector4f &pt_proj);

      /** \brief Get a string representation of the name of this class. */
      std::string 
      getName () const { return ("SampleConsensusModelCylinder"); }

    protected:
      /** \brief Check whether a model is valid given the user constraints.
        * \param[in] model_coefficients the set of model coefficients
        */
      bool 
      isModelValid (const Eigen::VectorXf &model_coefficients);

      /** \brief Check if a sample of indices results in a good sample of points
        * indices. Pure virtual.
        * \param[in] samples the resultant index samples
        */
      bool
      isSampleGood (const std::vector<int> &samples) const;

    private:
      /** \brief The axis along which we need to search for a plane perpendicular to. */
      Eigen::Vector3f axis_;
    
      /** \brief The maximum allowed difference between the plane normal and the given axis. */
      double eps_angle_;

      /** \brief Temporary boost mutex for \a tmp_inliers_ */
      boost::mutex tmp_mutex_;

      /** \brief temporary pointer to a list of given indices for optimizeModelCoefficients () */
      const std::vector<int> *tmp_inliers_;

      /** \brief Cost function to be minimized
        * \param[in] p a pointer to our data structure array
        * \param[in] m the number of functions
        * \param[in] n the number of variables
        * \param[in] x a pointer to the variables array
        * \param[out] fvec a pointer to the resultant functions evaluations
        * \param[in] iflag set to -1 inside the function to terminate execution
        */
      static int 
      functionToOptimize (void *p, int m, int n, const double *x, double *fvec, int iflag);
  };
}

#endif  //#ifndef PCL_SAMPLE_CONSENSUS_MODEL_CYLINDER_H_
