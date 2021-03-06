#pragma once

#include <memory>
#include <vector>
#include <memory.h>
#include <string>
#include "visualization/image_buffer.h"
#include "common/config.h"
#include "math/array_3d.h"
#include "common/interface.h"
#include "math/qr_svd/qr_svd.h"

TC_NAMESPACE_BEGIN

    inline void svd(const Matrix3 & A, Matrix3 & u, Matrix3 & sig, Matrix3 & v) {
        return imp_svd(transpose(A), u, sig, v);
    }

    inline void polar_decomp(const Matrix3 & A, Matrix3 & r, Matrix3 & s) {
        Matrix3 u, sig, v;
        svd(A, u, sig, v);
        r = u * glm::transpose(v);
        s = v * sig * glm::transpose(v);
		if (!is_normal(r)) {
			P(A);
			P(u);
			P(sig);
			P(v);
		}
    }

    class ParticleShadowMapRenderer;

    inline real det(const Matrix3 &m) {
        return glm::determinant(m);
    }

    class MPM3D: public Simulator {
    protected:
        typedef Vector3 Vector;
        typedef Matrix3 Matrix;
        typedef Region3D Region;
    public:
        static const int D = 3;

    public:
        struct Particle {
            using Vector = MPM3D::Vector;
            using Matrix = MPM3D::Matrix;
            using Region = MPM3D::Region;
            static const int D = MPM3D::D;
            Vector3 color = Vector3(1, 0, 0);
            Vector pos, v;
            Matrix dg_e, dg_p, tmp_force;
            real mass;
            real vol;
            Matrix apic_b;
            Matrix dg_cache;
            static long long instance_count;
            long long id = instance_count++;
            Particle() {
                dg_e = Matrix(1.0f);
                dg_p = Matrix(1.0f);
                apic_b = Matrix(0);
                v = Vector(0.0f);
                vol = 1.0f;
            }
            virtual void set_compression(float compression) {
                dg_p = Matrix(compression); // 1.0f = no compression
            }
            virtual Matrix get_energy_gradient() = 0;
            virtual void calculate_kernels() {}
            virtual void calculate_force() = 0;
            virtual void plasticity() {};
            virtual void print() {
                P(pos);
                P(v);
                P(dg_e);
                P(dg_p);
            }
            virtual ~Particle() {}
        };
        std::shared_ptr<ParticleShadowMapRenderer> particle_renderer;
        std::vector<Particle *> particles; // for efficiency
        Array3D<Vector> grid_velocity;
        Array3D<real> grid_mass;
        int width;
        int height;
        int depth;
        int max_dim;
        float t;
        Vector gravity;
        real delta_t;
        int substeps = 10;
        real viewport_rotation;

        Region get_bounded_rasterization_region(Vector p) {
            assert_info(is_normal(p.x) && is_normal(p.y) && is_normal(p.z), std::string("Abnormal p: ") + std::to_string(p.x)
                + ", " + std::to_string(p.y) + ", " + std::to_string(p.z));
            int x = int(p.x);
            int y = int(p.y);
            int z = int(p.z);
            /*
            int x_min = max(0, x - 1);
            int x_max = min(width, x + 3);
            int y_min = max(0, y - 1);
            int y_max = min(height, y + 3);
            int z_min = max(0, z - 1);
            int z_max = min(depth, z + 3);
            */
            int x_min = max(0, min(width, x - 1));
            int x_max = max(0, min(width, x + 3));
            int y_min = max(0, min(height, y - 1));
            int y_max = max(0, min(height, y + 3));
            int z_min = max(0, min(depth, z - 1));
            int z_max = max(0, min(depth, z + 3));
            return Region(x_min, x_max, y_min, y_max, z_min, z_max);
        }

        void estimate_volume() {}

        void rasterize();

        void resample(float delta_t);

        void apply_deformation_force(float delta_t);

        void apply_boundary_conditions();

        void apply_external_impulse(Vector impulse) {
            for (auto &p : particles) {
                p->v += impulse;
            }
        }

        void substep(float delta_t);

    public:

        MPM3D() {}

        void initialize(const Config &config);

        void step(real delta_t=0.0f) {
            real substep_delta_t = delta_t / substeps;
            for (int i = 0; i < substeps; i++) {
                substep(substep_delta_t);
            }
        }

        ImageBuffer<Vector3> get_visualization(int width, int height);

        void add_particle(const Config &config) {

        }

        float get_current_time() {
            return t;
        }

        ~MPM3D() {
            for (auto &p : particles) {
                delete p;
            }
        }

        //virtual boost::python::dict get_simulation_data(const Config &config);
    };

    std::shared_ptr<MPM3D> get_mpm_3d_simulator(const Config &config);
TC_NAMESPACE_END

