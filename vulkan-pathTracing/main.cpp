#include <glm/glm.hpp>

#include "Camera.h"
#include "ctmRand.h"
#include "HitableCollection.h"
#include "LogMessage.h"
#include "Material.h"
#include "PathTracing.h"
#include "Sphere.h"
#include "WindowApplication.h"

constexpr int WIDTH = 1080;
constexpr int HEIGHT = 720;
constexpr uint32_t NBR_SAMPLE = 8;

struct Color
{
	unsigned char b;
	unsigned char g;
	unsigned char r;
	unsigned char a;
};

namespace
{
	IHitable **random_scene()
	{
		int i = 0;
		IMaterial *material = nullptr;
		IHitable **list = new IHitable*[490];
		memset(list, 0, 450 * sizeof(IHitable));

		material = new Lambert(glm::vec3(0.5, 0.5, 0.5));
		list[i] = new Sphere(glm::vec3(0, -1000, 0), 1000, material);
		i++;

		for (int a = -11; a < 11; a++)
		{
			for (int b = -11; b < 11; b++)
			{
				float choose_mat = ctmRand();
				glm::vec3 center(a + 0.9 * ctmRand(), 0.2, b + 0.9 * ctmRand());
				if ((center - glm::vec3(4, 0.2, 0)).length() > 0.9)
				{
					if (choose_mat < 0.8)
					{
						material = new Lambert(glm::vec3(ctmRand() * ctmRand(),
							ctmRand() * ctmRand(),
							ctmRand() * ctmRand()));
					}
					else if (choose_mat < 0.95)
					{
						material = new Metal(glm::vec3(0.5f * (1.0f + ctmRand()),
							0.5f * (1.0f + ctmRand()),
							0.5f * (1.0f + ctmRand())), 0.5f * ctmRand());
					}
					else
					{
						material = new Dialectric(1.5);
					}
					list[i] = new Sphere(center, 0.2f, material);
					i++;
				}
			}
		}
		material = new Dialectric(1.3f);
		list[i] = new Sphere(glm::vec3(0, 1, 0), 1, material);
		i++;

		material = new Lambert(glm::vec3(0.4, 0.2, 0.1));
		list[i] = new Sphere(glm::vec3(-4, 1, 0), 1, material);
		i++;

		material = new Metal(glm::vec3(0.7, 0.6, 0.5), 0);
		list[i] = new Sphere(glm::vec3(4, 1, 0), 1, material);
		i++;

		list[i] = nullptr;
		LOG_MSG("%d", i);
		return list;
	}
}

int main()
{
	try
	{
		HitableCollection collection;
		collection.takeOwnershipOf(random_scene());

		glm::vec3 lookFrom(13, 2, 3);
		glm::vec3 lookAt(0, 0, 0);
		float dist_to_focus = 10;
		float aperture = 0.1f;
		Camera cam(lookFrom, lookAt, glm::vec3(0, 1, 0), 20, static_cast<float>(WIDTH) / HEIGHT, aperture, dist_to_focus);

		PathTracing pathTracing(WIDTH, HEIGHT, NBR_SAMPLE, collection, cam);
		WindowApplication winApp(WIDTH, HEIGHT);

		pathTracing.startRendering();
		while (winApp.isWindowOpen())
		{
			pathTracing.retreiveThreadResult();
			if (winApp.startFrame())
			{
				Color *pixels = reinterpret_cast<Color *>(winApp.getCurrentBuffer());

				const glm::vec3 *pic = pathTracing.getPic();

				for (uint32_t i = 0; i < WIDTH * HEIGHT; i++)
				{
					uint32_t x = i % WIDTH;
					uint32_t vy = i / WIDTH; // Vulkan image buffer index
					uint32_t py = HEIGHT - i / WIDTH - 1; // Pathtracing image buffer index

					pixels[x + vy * WIDTH].a = 255;
					pixels[x + vy * WIDTH].r = static_cast<unsigned char>(pic[x + py * WIDTH][0] * 255.99);
					pixels[x + vy * WIDTH].g = static_cast<unsigned char>(pic[x + py * WIDTH][1] * 255.99);
					pixels[x + vy * WIDTH].b = static_cast<unsigned char>(pic[x + py * WIDTH][2] * 255.99);
				}
				winApp.render();
			}
		}
		pathTracing.endRendering();
	}
	catch (std::exception &e)
	{
		LOG_CRIT(e.what());
	}
	return (0);
}