/* -- Fossae -- */

#include Library_Map

func InitializeMap(proplist map)
{
    Resize(120, 90);

    // Ground
    var ground_base = this->MapShapeSinus(6, 74, 0, 55, nil, 15);
    var ground_shape = this->MapShapeTurbulence(ground_base, 10);

    // Actual map
    this->Draw("Earth-earth", ground_shape);
    // Additional materials
    this->DrawSpots("Rock-rock", 5, [10, 30], [8, 12], nil, nil, ["Oil"]);
    this->DrawSpots("Crystal-crystal", 4, [8, 12], [4, 8], this->RectangleRelative(0, 80, 100, 20));
    this->DrawSpots("Oil-oil", 4, [3, 6], [8, 14], this->RectangleRelative(10, 70, 80, 20));
    // Make sure liquids don't border tunnel or sky sideways
    FixLiquidBorders();
    return true;
}


func MapShapeSinus(int amplitude, int period, int offset_x, int offset_y, array rect, int border)
{
    if (!rect) rect = [0, 0, this.Wdt, this.Hgt];
    var points_x = [rect[0] + rect[2] + border, rect[0] - border];
    var points_y = [rect[3] + border, rect[3] + border];
    
    offset_x = rect[2] * offset_x / 100;    
    offset_y = rect[3] * offset_y / 100;
    
    for (var x = 0; x < rect[2]; ++x)
    {
        PushBack(points_x, rect[0] + x);
        PushBack(points_y, CalcSinus(offset_x + x, amplitude, period, offset_y));
    }
    
    return {Algo = MAPALGO_Polygon, X = points_x, Y = points_y};
}


func MapShapeTurbulence(proplist algo, int turbulence)
{
   return {Algo=MAPALGO_Turbulence, Amplitude=turbulence ?? 10, Op = algo};
}


func CalcSinus(int x, int amplitude, int period, int offset_y)
{
    amplitude = amplitude ?? 10;
    period = period ?? 20;
    var angle = (x % period) * 360 / period;
    return offset_y + Sin(angle, amplitude);
}

func RectangleRelative(int percent_x, int percent_y, int percent_width, int percent_height)
{
    var percent = 100;
    var zoom = 1;
    percent_width = percent_width ?? percent;
    percent_height = percent_height ?? percent;
    var rect =  [zoom * percent_x * this.Wdt / percent,
                 zoom * percent_y * this.Hgt / percent,
                 zoom * percent_width * this.Wdt / percent,
                 zoom * percent_height * this.Hgt / percent];
    Log("Relative rectangle: %v", rect);
    return rect;
}

