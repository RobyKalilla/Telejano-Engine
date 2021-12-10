
#ifdef FLARES_T

#else

#define FLARES_T

typedef struct {
	int					active;
	vec3_t				origin;
	int					radius;
	int					mode;
	int					alfa;
} flares_t;

#endif