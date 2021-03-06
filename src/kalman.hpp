#ifndef KALMAN_H_
#define KALMAN_H_

#include "feature.hpp"   //Feature
#include "camera.hpp"   //Feature
#include "motion_model.hpp"   //Motion model
#include "print.hpp"

#include <Eigen/Dense> //Matrix

#include <opencv2/core/core.hpp> //Point2f

#include <Eigen/Core>  //Derived
#include <Eigen/Eigen> //math
#include <iostream>    //cout
#include <vector>   //vector


struct Features_extra{
	bool is_valid;
	cv::Point2f z_cv; //the feature actual observation coordinates as an openCV point
	Eigen::Vector2d z; //the feature actual observation coordinates
	Eigen::Vector2d h; //the feature state estimation represented in image coordinates
	Eigen::MatrixXd H; //the feature derivative against the current state (x_k_k)
};


class Kalman {
public:
	Kalman(double v_0, double std_v_0, double w_0, double std_w_0, double sigma_a, double sigma_alpha, double sigma_image_noise);
	Kalman(const Eigen::VectorXd & x_k_k, const Eigen::MatrixXd & p_k_k, double sigma_a, double sigma_alpha, double sigma_image_noise);

	static bool is_feature_valid(const Features_extra & feature_extra){
	    return feature_extra.is_valid == false;
	}
	void delete_features(std::vector<Features_extra> & features_extra);
	void predict_state_and_covariance(const double delta_t);
	void add_features_inverse_depth(const Camera & cam, const std::vector<cv::Point2f> & new_features_uvd_list);
	void set_state_position_value(const int index, const double value){
		x_k_k_(index) = value;
	}
	void compute_features_h(const Camera & cam, std::vector<Features_extra> & features_extra);
	void update(const Camera & cam, std::vector<Features_extra> & features_extra);
	Eigen::VectorXd x_k_k() const { return x_k_k_; }
	Eigen::MatrixXd p_k_k() const { return p_k_k_; }

private:

	double std_a_;     //standar deviation for linear acceleration noise
	double std_alpha_; //standar deviation for angular acceleration noise
	double std_z_;     //standar deviation for measurement noise

	Eigen::VectorXd x_k_k_;    //State vector
	Eigen::MatrixXd p_k_k_;    //Covariance matrix

	void add_a_feature_state_inverse_depth( const Eigen::VectorXd & XYZ_w, const int insert_point);

	void add_a_feature_covariance_inverse_depth( const Camera & cam, const Eigen::Vector2d & uvd, const Eigen::Vector3d & undistorted_projection, const Eigen::Vector4d & qWR, const Eigen::Matrix3d & qWR_rotation_matrix , const Eigen::Vector3d & XYZ_w, const int insert_point );


	void dR_by_dqw(const double qw, const double qx, const double qy, const double qz, Eigen::Matrix3d & m){
		m << 2*qw, -2*qz,  2*qy,
			 2*qz,  2*qw, -2*qx,
			-2*qy,  2*qx,  2*qw;
	}


	void dR_by_dqx(const double qw, const double qx, const double qy, const double qz, Eigen::Matrix3d & m){
		m << 2*qx,  2*qy,   2*qz,
			 2*qy, -2*qx,  -2*qw,
			 2*qz,  2*qw,  -2*qx;
	}


	void dR_by_dqy(const double qw, const double qx, const double qy, const double qz, Eigen::Matrix3d & m){
		m << -2*qy, 2*qx,  2*qw,
			  2*qx, 2*qy,  2*qz,
			 -2*qw, 2*qz, -2*qy;
	}

	void dR_by_dqz(const double qw, const double qx, const double qy, const double qz, Eigen::Matrix3d & m){
		m << -2*qz, -2*qw, 2*qx,
			  2*qw, -2*qz, 2*qy,
			  2*qx,  2*qy, 2*qz;
	}

	void normalize_jac(const Eigen::Vector4d & q, Eigen::Matrix4d & J){
		double w=q(0);
		double x=q(1);
		double y=q(2);
		double z=q(3);

		double t = w*w+x*x+y*y+z*z;
		J << x*x+y*y+z*z, 		 -w*x,		  -w*y, -w*z,
					-x*w, w*w+y*y+z*z,		  -x*y, -x*z,
					-y*w,		 -y*x, w*w+x*x+z*z, -y*z,
				-z*w, -z*x, -z*y,  w*w+x*x+y*y;
	    J *= 1/sqrt(t*t*t);
	}
};

#endif
