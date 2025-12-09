# MyGIS

A desktop GIS application built with Qt and GDAL for vector and raster data visualization, editing, and spatial analysis.

## Features

### Data Support
- **Vector**:  Shapefile (.shp), GeoJSON (.geojson), CSV (.csv)
- **Raster**: GeoTIFF (. tif), IMG (.img), PNG, JPEG
- **Coordinate System**: Auto-conversion to WGS84
- **Project Management**: Save/load GIS projects (.gproj)

### Vector Operations
- Interactive editing (point, line, polygon)
- Convex Hull calculation
- Buffer analysis
- Delaunay triangulation
- Attribute table view/edit
- Export to CSV/Database

### Raster Processing
- True color and grayscale display
- Statistical analysis
- Histogram generation
- Neighborhood statistics
- Color mapping
- Raster masking with vector layers

### Map View
- Pan, zoom, rotate
- Feature selection
- Layer management
- Auto-fit view

## Tech Stack

- C++
- Qt 5/6 (QtWidgets, QtCore, QtGui)
- GDAL/OGR
- log4cpp
- OpenMP
- Visual Studio

## Installation

### Prerequisites
- Visual Studio 2019+
- Qt 5.x or 6.x with Qt VS Tools
- GDAL library
- log4cpp

### Build
```bash
git clone https://github.com/fywoo32/MyGIS. git
cd MyGIS
# Open MyGIS.sln in Visual Studio
# Build and run
```

## Quick Start

1. **Load Data**:  File → Load Vector/Raster Layer
2. **Edit Features**: Select layer → Start Editing → Drag nodes → Stop Editing
3. **Spatial Analysis**: Tools → Buffer/Convex Hull/Delaunay
4. **Save Project**: File → Save Project

## Project Structure

```
MyGIS/
├── MyGIS/              # Main source code
│   ├── MyGIS.*         # Main window
│   ├── MyLayer.h       # Layer base class
│   ├── MyLayerManager. h # Layer manager
│   ├── Buffer.*        # Buffer analysis
│   ├── ConvexHull.*    # Convex hull
│   ├── Delaunay.*      # Delaunay triangulation
│   ├── Histogram.*     # Histogram
│   ├── Mask.*          # Raster masking
│   └── thirdlib/       # Third-party libraries
└── MyGIS.sln           # VS solution
```

## Core Modules

- **MyGIS**: Main application window and controller
- **MyLayerManager**:  Manages vector and raster layers
- **MyGraphicsView**: Custom graphics view with pan/zoom/rotate
- **MyGraphicsItem**:  Editable point/line/polygon items

## Notes

- Use "Load Large Raster" for large raster files
- Stop editing to save modifications
- Logs are saved to `Log.log`
- Some features require proper coordinate system information

## License

See LICENSE file for details. 

## Author

[@fywoo32](https://github.com/fywoo32)
