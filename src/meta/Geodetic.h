#pragma once
#include <type_traits>
#include <cmath>
#include <string>
#include <ostream>
#include <iostream>
#include "StrongUnits.h"
#include "ReyEngine.h"

namespace Geodetic {
   template <typename T>
   struct Printable {
      friend std::ostream &operator<<(std::ostream &os, const T &frame) {
         os << frame.print();
         return os;
      }
   };

   struct LatLon : public Printable<LatLon>{
      constexpr LatLon(Degrees lat, Degrees lon, MetersHAE alt): lat(lat), lon(lon), alt(alt){}
      inline bool operator==(const LatLon& other) const {return lat == other.lat && lon == other.lon && alt == other.alt;}
      inline bool operator!=(const LatLon& other) const {return !((*this)== other);}
      virtual std::string print() const {
         std::string s;
         return s + "[" + std::to_string(lat.get()) + "," + std::to_string(lon.get()) + "," + std::to_string(alt.get()) + "]";
      }
      Degrees lat;
      Degrees lon;
      MetersHAE alt;
   };

   struct ECEF : public Printable<ECEF> {
      constexpr ECEF(): x(0), y(0), z(0){};
      constexpr ECEF(const ReyEngine::Vec3<Meters>& v): x(x), y(y), z(z){}
      constexpr ECEF(Meters x, Meters y, Meters z): x(x), y(y), z(z){}
      Meters x, y, z;
      std::string print() const {
         std::string s;
         return s + "[" + std::to_string(x.get()) + "," + std::to_string(y.get()) + "," + std::to_string(z.get()) + "]";
      }

      inline explicit operator ReyEngine::Vec3<Meters>() const {return {x.get(),y.get(),z.get()};}
      ReyEngine::Vec3<Meters> operator-(const ECEF& other) const {
         auto a = (ReyEngine::Vec3<Meters>)*this;
         auto b = (ReyEngine::Vec3<Meters>)other;
         return b - a;
      }
   };

   ///East = x, North = y, Up = z convention
   struct ENU;
   inline ECEF enu_to_ecef(const ENU &enu);
   struct ENU : public Printable<ENU> {
      constexpr ENU(const LatLon& basis): origin(basis), e(0), n(0), u(0){}
      constexpr ENU(const LatLon& basis, Meters east, Meters north, Meters up): origin(basis), e(east), n(north), u(up){}
      Meters e;
      Meters n;
      Meters u;
      const LatLon origin;
      std::string print() const {std::string s;return s + "[" + std::to_string(e.get()) + "," + std::to_string(n.get()) + "," + std::to_string(u.get()) + " relative to " + origin.print() + "]";}
      bool hasOffset(){return e.get() || n.get() || u.get();}
      bool hasOffset(Meters _e, Meters _n, Meters _u){return  e==_e && n==_n && u==_u;}
      template <typename T> explicit inline operator ReyEngine::Vec3<T>() const {return ReyEngine::Vec3<T>(e.get(),n.get(),u.get(), 1);}
      template <typename T> ReyEngine::Vec3<T> operator-(const ENU& rhs) const{
         //for now, assumes the origins are the same
         if (origin == rhs.origin){
            return ReyEngine::Vec3<T>(*this) - ReyEngine::Vec3<T>(rhs);
         }
         //rebase points to have same origin
         auto _lhs = enu_to_ecef(*this);
         auto _rhs = enu_to_ecef(rhs);

         //subtract the two points so we have a difference vector
         return _lhs - _rhs;
      };
   };

   namespace Internal {
      template<typename Source, typename Dest, Dest (*FunctorSourceDest)(const Source &), Source (*FunctorDestSource)(
            const Dest &) = nullptr>
      struct FrameTransform {
      protected:
         static Dest SourcetoDest(const Source &a) {
            return FunctorSourceDest(a);
         };

         static Source DestToSource(const Dest &dest) {
            static_assert(FunctorDestSource != nullptr,
                          "You did not define a Dest->Source conversion function! Pass it to the template!");
            return FunctorDestSource(dest);
         };

      public:
         inline Dest operator<<(const Source &source) {return SourcetoDest(source);}
         inline Source operator<<(const Dest &dest) {return DestToSource(dest);}
         static Dest convert(const Source &source) {return SourcetoDest(source);}
         static Source convert(const Dest &dest) {return DestToSource(dest);}
         inline FrameTransform& operator<<(const FrameTransform& other) {
            other << *this;
            return *this;
         }
      };

      static constexpr double WGS84_A  = 6378137.0;               // Semi-major axis (meters)
      static constexpr double WGS84_E2 = 6.69437999014e-3;        // First eccentricity squared
      inline ECEF wgs84_to_ecef(const LatLon &wgs84) {
         // Convert degrees to radians
         double lat_rad = Radians(wgs84.lat).get();
         double lon_rad = Radians(wgs84.lon).get();
         double alt = wgs84.alt.get();

         // Calculate prime vertical radius of curvature
         double N = WGS84_A / std::sqrt(1.0 - WGS84_E2 * std::sin(lat_rad) * std::sin(lat_rad));

         // Calculate ECEF coordinates
         double x = (N + alt) * std::cos(lat_rad) * std::cos(lon_rad);
         double y = (N + alt) * std::cos(lat_rad) * std::sin(lon_rad);
         double z = ((1 - WGS84_E2) * N + alt) * std::sin(lat_rad);
         return {x, y, z};
      }

      inline LatLon ecef_to_wgs84(const ECEF &ecef) {
         const double x = ecef.x.get();
         const double y = ecef.y.get();
         const double z = ecef.z.get();

         // Longitude (radians)
         double lon = std::atan2(y, x);

         // Iteratively calculate latitude (radians)
         double p = std::sqrt(x * x + y * y);
         double lat = std::atan2(z, p * (1 - WGS84_E2));  // Initial guess

         double N;
         double prev_lat;
         do {
            prev_lat = lat;
            N = WGS84_A / std::sqrt(1 - WGS84_E2 * std::sin(lat) * std::sin(lat));
            lat = std::atan2(z + WGS84_E2 * N * std::sin(prev_lat), p);
         } while (std::abs(lat - prev_lat) > 1e-12);  // Iterate until convergence

         // Altitude
         N = WGS84_A / std::sqrt(1 - WGS84_E2 * std::sin(lat) * std::sin(lat));
         double alt = p / std::cos(lat) - N;
         return LatLon(Radians(lat), Radians(lon), MetersHAE(alt));
      }

      inline ECEF enu_to_ecef(const ENU &enu) {
         // WGS84 ellipsoid parameters
         constexpr double a = 6378137.0;  // Semi-major axis in meters
         constexpr double e2 = 0.00669437999014;  // Square of eccentricity

         // Reference position in WGS84 (latitude, longitude, altitude)
         double lat0 = enu.origin.lat.get();
         double lon0 = enu.origin.lon.get();
         double alt0 = enu.origin.alt.get();

         // Convert lat/lon to radians
         lat0 = lat0 * M_PI / 180.0;
         lon0 = lon0 * M_PI / 180.0;

         // Compute the radius of curvature in the prime vertical
         double N = a / sqrt(1 - e2 * sin(lat0) * sin(lat0));

         // Convert reference WGS84 position to ECEF
         double X0 = (N + alt0) * cos(lat0) * cos(lon0);
         double Y0 = (N + alt0) * cos(lat0) * sin(lon0);
         double Z0 = (N * (1 - e2) + alt0) * sin(lat0);

         // ENU to ECEF conversion (Rotation and Translation)
         auto X = -sin(lon0) * enu.e - sin(lat0) * cos(lon0) * enu.n + cos(lat0) * cos(lon0) * enu.u + X0;
         auto Y = cos(lon0) * enu.e - sin(lat0) * sin(lon0) * enu.n + cos(lat0) * sin(lon0) * enu.u + Y0;
         auto Z = cos(lat0) * enu.n + sin(lat0) * enu.u + Z0;

         // Return the result as ECEF coordinates
         return ECEF(X, Y, Z);
      }

      inline ENU ecef_to_enu(const ECEF& ecef){
         return ENU(ecef_to_wgs84(ecef));
      }

      // Convert ECEF to ENU given the reference origin
      inline ENU ecefToENU(const ECEF &pointECEF, const LatLon &origin) {
         ECEF originECEF = wgs84_to_ecef(origin);

         double latRad = Radians(origin.lat).get();
         double lonRad = Radians(origin.lon).get();

         // Calculate the difference in ECEF coordinates
         double dx = pointECEF.x.get() - originECEF.x.get();
         double dy = pointECEF.y.get() - originECEF.y.get();
         double dz = pointECEF.z.get() - originECEF.z.get();

         // Calculate ENU using the transformation matrix
         double east  = -std::sin(lonRad) * dx + std::cos(lonRad) * dy;
         double north = -std::sin(latRad) * std::cos(lonRad) * dx
                        - std::sin(latRad) * std::sin(lonRad) * dy
                        + std::cos(latRad) * dz;
         double up    =  std::cos(latRad) * std::cos(lonRad) * dx
                         + std::cos(latRad) * std::sin(lonRad) * dy
                         + std::sin(latRad) * dz;

         return {origin, east, north, up};
      }
   }
   class LatLonEcef : public Internal::FrameTransform<LatLon, ECEF, Internal::wgs84_to_ecef, Internal::ecef_to_wgs84>{};
   class EnuEcef : public Internal::FrameTransform<ENU, ECEF, Internal::enu_to_ecef, Internal::ecef_to_enu>{};
}