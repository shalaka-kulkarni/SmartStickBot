
#include <iostream>
#include <string.h>
#include <vector>

using namespace std;

struct LatLng {
    double lat=0, lng=0;
};


vector<LatLng> polyline;
void decodePolyline(string encoded) {
    polyline.clear();

    int index = 0, len = encoded.length();
    int lat = 0, lng = 0;

    while (index < len) {
        int b, shift = 0, result = 0;
        do {
            b = encoded[index++] - 63;
            result |= (b & 0x1f) << shift;
            shift += 5;
        } while (b >= 0x20);
        int dlat = ((result & 1) != 0 ? ~(result >> 1) : (result >> 1));
        lat += dlat;

        shift = 0;
        result = 0;
        do {
            b = encoded[index++] - 63;
            result |= (b & 0x1f) << shift;
            shift += 5;
        } while (b >= 0x20);
        int dlng = ((result & 1) != 0 ? ~(result >> 1) : (result >> 1));
        lng += dlng;

        LatLng p;

        p.lat= (double) lat / 100000; p.lng= (double) lng / 100000;

        polyline.push_back(p);
    }
}

int main()
{
    //url=https://maps.googleapis.com/maps/api/directions/json?origin=IITB+main+gate&destination=Sunrise+Dhaba
    cout<<"IIT Main Gate to SAC : Latitude/Longitude of step points along the route"<<endl;
    string poly = "ujvsBsp`|LSTKFEEyAmAiBn@gAmA_IjCg@PK@cFIC}AKCGAETYt@_@v@GB_ADyCN_@@D|CDl@_CXQB_@Ni@XMS]Z_@l@Ur@[zAuMuCeCi@NlABX";
    decodePolyline(poly);
    for(int i =0; i<polyline.size();i++)
    {
        cout<<i+1<<" step "<<polyline[i].lat<<' '<<polyline[i].lng<<endl;
    }
}

