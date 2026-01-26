#include <libgeom.h>
#include <stdio.h>
#include <string.h>
#include <sys/queue.h>

int main() {
  struct gmesh mesh;
  struct gclass *classp;
  struct ggeom *geomp;
  struct gprovider *providerp;
  geom_gettree(&mesh);

  printf("Name  Size\n");
  LIST_FOREACH(classp, &mesh.lg_class, lg_class) {
    if (strcmp(classp->lg_name, "DISK") != 0 &&
        strcmp(classp->lg_name, "PART") != 0)
      continue;

    LIST_FOREACH(geomp, &classp->lg_geom, lg_geom) {
      LIST_FOREACH(providerp, &geomp->lg_provider, lg_provider) {
        printf("%s %lld\n", providerp->lg_name,
               (unsigned long long)providerp->lg_mediasize);
      }
    }
  }

  geom_deletetree(&mesh);
}
