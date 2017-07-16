FROM margrietpalm/visgrid3d-build AS build

WORKDIR /app
COPY . /app/VisGrid3D

# build VisGrid3D
RUN mkdir -p VisGrid3D/build/
RUN cd VisGrid3D/build/ && cmake ../ && make


FROM visgrid3d-build
COPY --from=build /app/VisGrid3D/build/VisGrid3D /app/VisGrid3D

WORKDIR /app
ENTRYPOINT ["/app/VisGrid3D"]
