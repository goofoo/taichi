#include "camera.h"

TC_NAMESPACE_BEGIN
    TC_INTERFACE_DEF(Camera, "Camera");

    /*
    class OrthoCamera : public Camera {
    public:

        OrthoCamera(Vector3 origin, Vector3 look_at, Vector3 up, Vector2 viewport_size) {
            error("no impl");
            this->origin = origin;
            this->look_at = look_at;
            this->viewport_size = viewport_size;
            this->up = up;
            set_dir_and_right();
        }

    private:
        Vector2 viewport_size;

        virtual Ray sample(Vector2 offset, Vector2 size, real u, real v) override {
            Vector2 offset_sampled_world = random_offset(offset, size, u, v) * viewport_size;
            Vector3 offset3 = offset_sampled_world.x * right + offset_sampled_world.y * up;
            return Ray(origin + offset3, dir, 0.0f);
        }
    };
    */

    class PerspectiveCamera : public Camera {
    public:

        PerspectiveCamera() {}

        virtual void initialize(const Config &config) override;

        void initialize(ptree &t, real aspect_ratio) override;

        void initialize(Vector3 origin, Vector3 look_at, Vector3 up, real fov_angle,
                        const Matrix4 &transform, real aspect_ratio);

        real get_pixel_scaling() override;

        virtual Ray sample(Vector2 offset, Vector2 size, real u, real v) override;

        void get_pixel_coordinate(Vector3 ray_dir, real &u, real &v) override;

    private:
        real fov;
        real tan_half_fov;
        real aspect_ratio;
    };
    void PerspectiveCamera::initialize(const Config &config) {
        this->initialize(config.get_vec3("origin"), config.get_vec3("look_at"),
                   config.get_vec3("up"), config.get_real("fov_angle"), Matrix4(1.0f), config.get_real("aspect_ratio"));
    }

    void PerspectiveCamera::initialize(ptree &t, real aspect_ratio) {
        auto trans = load_matrix4(t.get_child("transform"));
        initialize(Vector3(0, 0, 0), Vector3(0, 0, -1), Vector3(0, 1, 0), 60.0f, trans, aspect_ratio);
    }

    void PerspectiveCamera::initialize(Vector3 origin, Vector3 look_at, Vector3 up, real fov_angle,
                                       const Matrix4 &transform, real aspect_ratio) {
        fov = fov_angle / 180.0f * pi;
        this->origin = origin;
        this->look_at = look_at;
        this->up = up;
        set_dir_and_right();
        tan_half_fov = tan(fov / 2);
        this->aspect_ratio = aspect_ratio;
        this->transform = transform;
    }

    real PerspectiveCamera::get_pixel_scaling() {
        return sqr(tan_half_fov) * aspect_ratio;
    }

    Ray PerspectiveCamera::sample(Vector2 offset, Vector2 size, real u, real v) {
        Vector2 rand_offset = random_offset(offset, size, u, v);
        Vector3 local_dir = normalize(
                dir + rand_offset.x * tan_half_fov * right * aspect_ratio + rand_offset.y * tan_half_fov * up);
        Vector3 world_orig = multiply_matrix4(transform, origin, 1);
        Vector3 world_dir = normalized(multiply_matrix4(transform, local_dir, 0)); //TODO: why normalize here???
        return Ray(world_orig, world_dir, 0);
    }

    void PerspectiveCamera::get_pixel_coordinate(Vector3 ray_dir, real &u, real &v) {
        auto inv_transform = glm::inverse(transform);
        auto local_ray_dir = multiply_matrix4(inv_transform, ray_dir, 0);
        u = dot(local_ray_dir, right) / dot(local_ray_dir, dir) / tan_half_fov / aspect_ratio + 0.5f;
        v = dot(local_ray_dir, up) / dot(local_ray_dir, dir) / tan_half_fov + 0.5f;
    }
    TC_IMPLEMENTATION(Camera, PerspectiveCamera, "perspective");


TC_NAMESPACE_END

