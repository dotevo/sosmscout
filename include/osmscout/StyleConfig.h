#ifndef OSMSCOUT_STYLECONFIG_H
#define OSMSCOUT_STYLECONFIG_H

/*
  This source is part of the libosmscout library
  Copyright (C) 2009  Tim Teulings

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
*/

#include <vector>

#include <osmscout/Types.h>
#include <osmscout/TypeConfig.h>

namespace osmscout {

  /**
   * Ways can have a line style
   */
  class LineStyle
  {
  private:
    double              lineR; /**< TODO */
    double              lineG; /**< TODO */
    double              lineB; /**< TODO */
    double              lineA; /**< TODO */
    double              alternateR; /**< TODO */
    double              alternateG; /**< TODO */
    double              alternateB; /**< TODO */
    double              alternateA; /**< TODO */
    double              outlineR; /**< TODO */
    double              outlineG; /**< TODO */
    double              outlineB; /**< TODO */
    double              outlineA; /**< TODO */
    double              gapR; /**< TODO */
    double              gapG; /**< TODO */
    double              gapB; /**< TODO */
    double              gapA; /**< TODO */
    double              minPixel; /**< TODO */
    double              width; /**< TODO */
    double              fixedWidth; /**< TODO */
    double              outline; /**< TODO */
    std::vector<double> dash; /**< TODO */

  public:
/**
 * @brief
 *
 */
    LineStyle();

    /**
     * @brief
     *
     * @param r
     * @param g
     * @param b
     * @param a
     */
    LineStyle& SetLineColor(double r, double g, double b, double a);
    /**
     * @brief
     *
     * @param r
     * @param g
     * @param b
     * @param a
     */
    LineStyle& SetAlternateColor(double r, double g, double b, double a);
    /**
     * @brief
     *
     * @param r
     * @param g
     * @param b
     * @param a
     */
    LineStyle& SetOutlineColor(double r, double g, double b, double a);
    /**
     * @brief
     *
     * @param r
     * @param g
     * @param b
     * @param a
     */
    LineStyle& SetGapColor(double r, double g, double b, double a);
    /**
     * @brief
     *
     * @param value
     */
    LineStyle& SetMinPixel(double value);
    /**
     * @brief
     *
     * @param value
     */
    LineStyle& SetWidth(double value);
    /**
     * @brief
     *
     * @param fixedWidth
     */
    LineStyle& SetFixedWidth(bool fixedWidth);
    /**
     * @brief
     *
     * @param value
     */
    LineStyle& SetOutline(double value);
    /**
     * @brief
     *
     * @param dashValue
     */
    LineStyle& AddDashValue(double dashValue);

    /**
     * @brief
     *
     * @return bool
     */
    inline bool IsVisible() const
    {
      return width>0.0;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetLineR() const
    {
      return lineR;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetLineG() const
    {
      return lineG;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetLineB() const
    {
      return lineB;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetLineA() const
    {
      return lineA;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetAlternateR() const
    {
      return alternateR;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetAlternateG() const
    {
      return alternateG;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetAlternateB() const
    {
      return alternateB;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetAlternateA() const
    {
      return alternateA;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetOutlineR() const
    {
      return outlineR;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetOutlineG() const
    {
      return outlineG;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetOutlineB() const
    {
      return outlineB;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetOutlineA() const
    {
      return outlineA;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetGapR() const
    {
      return gapR;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetGapG() const
    {
      return gapG;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetGapB() const
    {
      return gapB;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetGapA() const
    {
      return gapA;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetMinPixel() const
    {
      return minPixel;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetWidth() const
    {
      return width;
    }

    /**
     * @brief
     *
     * @return bool
     */
    inline bool GetFixedWidth() const
    {
      return fixedWidth;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetOutline() const
    {
      return outline;
    }

    /**
     * @brief
     *
     * @return bool
     */
    inline bool HasDashValues() const
    {
      return dash.size()>0;
    }

    /**
     * @brief
     *
     * @return const std::vector<double>
     */
    inline const std::vector<double>& GetDash() const
    {
      return dash;
    }
  };

  /**
   * Areas can have a fill style, filling the area with one color
   */
  /**
   * @brief
   *
   */
  class FillStyle
  {
  public:
    /**
     * @brief
     *
     */
    enum Style {
      none,
      plain
    };

  private:
    Style               style; /**< TODO */
    int                 layer; /**< TODO */
    double              fillR; /**< TODO */
    double              fillG; /**< TODO */
    double              fillB; /**< TODO */
    double              fillA; /**< TODO */
    std::string         pattern; /**< TODO */
    mutable size_t      patternId; /**< TODO */
    Mag                 patternMinMag; /**< TODO */
    double              borderR; /**< TODO */
    double              borderG; /**< TODO */
    double              borderB; /**< TODO */
    double              borderA; /**< TODO */
    double              borderMinPixel; /**< TODO */
    double              borderWidth; /**< TODO */
    std::vector<double> borderDash; /**< TODO */

  public:
/**
 * @brief
 *
 */
    FillStyle();

    /**
     * @brief
     *
     * @param style
     */
    FillStyle& SetStyle(Style style);
    /**
     * @brief
     *
     * @param layer
     */
    FillStyle& SetLayer(int layer);
    /**
     * @brief
     *
     * @param r
     * @param g
     * @param b
     * @param a
     */
    FillStyle& SetFillColor(double r, double g, double b, double a);
    /**
     * @brief
     *
     * @param id
     */
    void SetPatternId(size_t id) const;
    /**
     * @brief
     *
     * @param pattern
     */
    FillStyle& SetPattern(const std::string& pattern);
    /**
     * @brief
     *
     * @param mag
     */
    FillStyle& SetPatternMinMag(Mag mag);
    /**
     * @brief
     *
     * @param r
     * @param g
     * @param b
     * @param a
     */
    FillStyle& SetBorderColor(double r, double g, double b, double a);
    /**
     * @brief
     *
     * @param value
     */
    FillStyle& SetBorderMinPixel(double value);
    /**
     * @brief
     *
     * @param value
     */
    FillStyle& SetBorderWidth(double value);
    /**
     * @brief
     *
     * @param dashValue
     */
    FillStyle& AddBorderDashValue(double dashValue);

    /**
     * @brief
     *
     * @return bool
     */
    inline bool IsVisible() const
    {
      return style!=none;
    }

    /**
     * @brief
     *
     * @return const Style &
     */
    inline const Style& GetStyle() const
    {
      return style;
    }

    /**
     * @brief
     *
     * @return int
     */
    inline int GetLayer() const
    {
      return layer;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetFillR() const
    {
      return fillR;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetFillG() const
    {
      return fillG;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetFillB() const
    {
      return fillB;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetFillA() const
    {
      return fillA;
    }

    /**
     * @brief
     *
     * @return bool
     */
    inline bool HasPattern() const
    {
      return !pattern.empty();
    }

    /**
     * @brief
     *
     * @return size_t
     */
    inline size_t GetPatternId() const
    {
      return patternId;
    }

    /**
     * @brief
     *
     * @return std::string
     */
    inline std::string GetPatternName() const
    {
      return pattern;
    }

    /**
     * @brief
     *
     * @return const Mag &
     */
    inline const Mag& GetPatternMinMag() const
    {
      return patternMinMag;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetBorderR() const
    {
      return borderR;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetBorderG() const
    {
      return borderG;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetBorderB() const
    {
      return borderB;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetBorderA() const
    {
      return borderA;
    }
    /**
     * @brief
     *
     * @return double
     */
    inline double GetBorderMinPixel() const
    {
      return borderMinPixel;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetBorderWidth() const
    {
      return borderWidth;
    }

    /**
     * @brief
     *
     * @return bool
     */
    inline bool HasBorderDashValues() const
    {
      return borderDash.size()>0;
    }

    /**
     * @brief
     *
     * @return const std::vector<double>
     */
    inline const std::vector<double>& GetBorderDash() const
    {
      return borderDash;
    }
  };

  /**
    Nodes, ways and areas can have a label style for drawing text. Text can be formatted
    in different ways.
   */
  /**
   * @brief
   *
   */
  class LabelStyle
  {
  public:
    /**
     * @brief
     *
     */
    enum Style {
      none,
      normal,
      contour,
      plate,
      emphasize
    };

  private:
    Style   style; /**< TODO */
    uint8_t priority; /**< TODO */
    Mag     minMag; /**< TODO */
    Mag     scaleAndFadeMag; /**< TODO */
    Mag     maxMag; /**< TODO */
    double  size; /**< TODO */
    double  textR; /**< TODO */
    double  textG; /**< TODO */
    double  textB; /**< TODO */
    double  textA; /**< TODO */
    double  bgR; /**< TODO */
    double  bgG; /**< TODO */
    double  bgB; /**< TODO */
    double  bgA; /**< TODO */
    double  borderR; /**< TODO */
    double  borderG; /**< TODO */
    double  borderB; /**< TODO */
    double  borderA; /**< TODO */

  public:
/**
 * @brief
 *
 */
    LabelStyle();

    /**
     * @brief
     *
     * @param style
     */
    LabelStyle& SetStyle(Style style);
    /**
     * @brief
     *
     * @param priority
     */
    LabelStyle& SetPriority(uint8_t priority);
    /**
     * @brief
     *
     * @param mag
     */
    LabelStyle& SetMinMag(Mag mag);
    /**
     * @brief
     *
     * @param mag
     */
    LabelStyle& SetScaleAndFadeMag(Mag mag);
    /**
     * @brief
     *
     * @param mag
     */
    LabelStyle& SetMaxMag(Mag mag);
    /**
     * @brief
     *
     * @param size
     */
    LabelStyle& SetSize(double size);
    /**
     * @brief
     *
     * @param r
     * @param g
     * @param b
     * @param a
     */
    LabelStyle& SetTextColor(double r, double g, double b, double a);
    /**
     * @brief
     *
     * @param r
     * @param g
     * @param b
     * @param a
     */
    LabelStyle& SetBgColor(double r, double g, double b, double a);
    /**
     * @brief
     *
     * @param r
     * @param g
     * @param b
     * @param a
     */
    LabelStyle& SetBorderColor(double r, double g, double b, double a);

    /**
     * @brief
     *
     * @return bool
     */
    inline bool IsVisible() const
    {
      return style!=none;
    }

    /**
     * @brief
     *
     * @return const Style &
     */
    inline const Style& GetStyle() const
    {
      return style;
    }

    /**
     * @brief
     *
     * @return bool
     */
    inline bool IsPointStyle() const
    {
      return style==normal || style==plate || style==emphasize;
    }

    /**
     * @brief
     *
     * @return bool
     */
    inline bool IsContourStyle() const
    {
      return style==contour;
    }

    /**
     * @brief
     *
     * @return uint8_t
     */
    inline uint8_t GetPriority() const
    {
      return priority;
    }

    /**
     * @brief
     *
     * @return Mag
     */
    inline Mag GetMinMag() const
    {
      return minMag;
    }

    /**
     * @brief
     *
     * @return Mag
     */
    inline Mag GetScaleAndFadeMag() const
    {
      return scaleAndFadeMag;
    }

    /**
     * @brief
     *
     * @return Mag
     */
    inline Mag GetMaxMag() const
    {
      return maxMag;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetSize() const
    {
      return size;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetTextR() const
    {
      return textR;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetTextG() const
    {
      return textG;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetTextB() const
    {
      return textB;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetTextA() const
    {
      return textA;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetBgR() const
    {
      return bgR;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetBgG() const
    {
      return bgG;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetBgB() const
    {
      return bgB;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetBgA() const
    {
      return bgA;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetBorderR() const
    {
      return borderR;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetBorderG() const
    {
      return borderG;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetBorderB() const
    {
      return borderB;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetBorderA() const
    {
      return borderA;
    }
  };

  /**
    Nodes and areas can have a symbol style.A symbol is a internal predefined simple
    iconic image, most of the time simple geometric forms lice circles, crosses and
    similar.
   */
  /**
   * @brief
   *
   */
  class SymbolStyle
  {
  public:
    /**
     * @brief
     *
     */
    enum Style {
      none,
      box,
      circle,
      triangle
    };

  private:
    Style  style; /**< TODO */
    Mag    minMag; /**< TODO */
    double size; /**< TODO */
    double fillR; /**< TODO */
    double fillG; /**< TODO */
    double fillB; /**< TODO */
    double fillA; /**< TODO */

  public:
/**
 * @brief
 *
 */
    SymbolStyle();

    /**
     * @brief
     *
     * @param style
     */
    SymbolStyle& SetStyle(Style style);
    /**
     * @brief
     *
     * @param mag
     */
    SymbolStyle& SetMinMag(Mag mag);
    /**
     * @brief
     *
     * @param size
     */
    SymbolStyle& SetSize(double size);
    /**
     * @brief
     *
     * @param r
     * @param g
     * @param b
     * @param a
     */
    SymbolStyle& SetFillColor(double r, double g, double b, double a);

    /**
     * @brief
     *
     * @return bool
     */
    inline bool IsVisible() const
    {
      return style!=none;
    }

    /**
     * @brief
     *
     * @return const Style &
     */
    inline const Style& GetStyle() const
    {
      return style;
    }

    /**
     * @brief
     *
     * @return const Mag &
     */
    inline const Mag& GetMinMag() const
    {
      return minMag;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetSize() const
    {
      return size;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetFillR() const
    {
      return fillR;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetFillG() const
    {
      return fillG;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetFillB() const
    {
      return fillB;
    }

    /**
     * @brief
     *
     * @return double
     */
    inline double GetFillA() const
    {
      return fillA;
    }
  };

  /**
    IconStyle is for define drawing of external images as icons for nodes and areas
    */
  /**
   * @brief
   *
   */
  class IconStyle
  {
  private:
    size_t      id;       //! Internal id for fast lookup. 0 == no id defined (yet), max(size_t) == error /**< TODO */
    std::string iconName; //! name of the icon as given in style /**< TODO */
    Mag         minMag;   //! minimum magnification to show icon /**< TODO */

  public:
/**
 * @brief
 *
 */
    IconStyle();

    /**
     * @brief
     *
     * @param id
     */
    IconStyle& SetId(size_t id);
    /**
     * @brief
     *
     * @param iconName
     */
    IconStyle& SetIconName(const std::string& iconName);
    /**
     * @brief
     *
     * @param mag
     */
    IconStyle& SetMinMag(Mag mag);

    /**
     * @brief
     *
     * @return bool
     */
    inline bool IsVisible() const
    {
      return !iconName.empty();
    }

    /**
     * @brief
     *
     * @return size_t
     */
    inline size_t GetId() const
    {
      return id;
    }

    /**
     * @brief
     *
     * @return std::string
     */
    inline std::string GetIconName() const
    {
      return iconName;
    }

    /**
     * @brief
     *
     * @return const Mag &
     */
    inline const Mag& GetMinMag() const
    {
      return minMag;
    }
  };

  /**
   * A complete style definition
   */
  /**
   * @brief
   *
   */
  class StyleConfig
  {
  private:
    TypeConfig                *typeConfig; /**< TODO */

    // Node

    std::vector<SymbolStyle*>  nodeSymbolStyles; /**< TODO */
    std::vector<LabelStyle*>   nodeRefLabelStyles; /**< TODO */
    std::vector<LabelStyle*>   nodeLabelStyles; /**< TODO */
    std::vector<IconStyle*>    nodeIconStyles; /**< TODO */

    // Way

    std::vector<LineStyle*>    wayLineStyles; /**< TODO */
    std::vector<LabelStyle*>   wayRefLabelStyles; /**< TODO */
    std::vector<LabelStyle*>   wayNameLabelStyles; /**< TODO */

    // Area

    std::vector<FillStyle*>    areaFillStyles; /**< TODO */
    std::vector<SymbolStyle*>  areaSymbolStyles; /**< TODO */
    std::vector<LabelStyle*>   areaLabelStyles; /**< TODO */
    std::vector<IconStyle*>    areaIconStyles; /**< TODO */

    std::vector<size_t>        wayPrio; /**< TODO */
    std::vector<Mag>           wayMag; /**< TODO */
    std::vector<Mag>           areaMag; /**< TODO */
    std::vector<TypeId>        wayTypesByPrio; /**< TODO */

  public:
/**
 * @brief
 *
 * @param typeConfig
 */
    StyleConfig(TypeConfig* typeConfig);
    /**
     * @brief
     *
     */
    virtual ~StyleConfig();

    /**
     * @brief
     *
     */
    void Postprocess();

    /**
     * @brief
     *
     */
    TypeConfig* GetTypeConfig() const;

    /**
     * @brief
     *
     * @param type
     * @param prio
     */
    StyleConfig& SetWayPrio(TypeId type, size_t prio);
    /**
     * @brief
     *
     * @param type
     * @param mag
     */
    StyleConfig& SetWayMag(TypeId type, Mag mag);
    /**
     * @brief
     *
     * @param type
     * @param mag
     */
    StyleConfig& SetAreaMag(TypeId type, Mag mag);

    /**
     * @brief
     *
     * @param type
     * @param style
     */
    StyleConfig& SetNodeSymbolStyle(TypeId type, const SymbolStyle& style);
    /**
     * @brief
     *
     * @param type
     * @param style
     */
    StyleConfig& SetNodeRefLabelStyle(TypeId type, const LabelStyle& style);
    /**
     * @brief
     *
     * @param type
     * @param style
     */
    StyleConfig& SetNodeLabelStyle(TypeId type, const LabelStyle& style);
    /**
     * @brief
     *
     * @param type
     * @param style
     */
    StyleConfig& SetNodeIconStyle(TypeId type, const IconStyle& style);

    /**
     * @brief
     *
     * @param type
     * @param style
     */
    StyleConfig& SetWayLineStyle(TypeId type, const LineStyle& style);
    /**
     * @brief
     *
     * @param type
     * @param style
     */
    StyleConfig& SetWayRefLabelStyle(TypeId type, const LabelStyle& style);
    /**
     * @brief
     *
     * @param type
     * @param style
     */
    StyleConfig& SetWayNameLabelStyle(TypeId type, const LabelStyle& style);

    /**
     * @brief
     *
     * @param type
     * @param style
     */
    StyleConfig& SetAreaFillStyle(TypeId type, const FillStyle& style);
    /**
     * @brief
     *
     * @param type
     * @param style
     */
    StyleConfig& SetAreaLabelStyle(TypeId type, const LabelStyle& style);
    /**
     * @brief
     *
     * @param type
     * @param style
     */
    StyleConfig& SetAreaSymbolStyle(TypeId type, const SymbolStyle& style);
    /**
     * @brief
     *
     * @param type
     * @param style
     */
    StyleConfig& SetAreaIconStyle(TypeId type, const IconStyle& style);

    /**
     * @brief
     *
     * @param mag
     * @param types
     */
    void GetNodeTypesWithMag(double mag,
                             std::vector<TypeId>& types) const;
    /**
     * @brief
     *
     * @param mag
     * @param types
     */
    void GetWayTypesByPrioWithMag(double mag,
                                  std::vector<TypeId>& types) const;
    /**
     * @brief
     *
     * @param mag
     * @param types
     */
    void GetAreaTypesWithMag(double mag,
                             TypeSet& types) const;


    /**
     * @brief
     *
     * @param type
     * @return size_t
     */
    inline size_t GetWayPrio(TypeId type) const
    {
      return wayPrio[type];
    }

    /**
     * @brief
     *
     * @param type
     * @return const SymbolStyle *
     */
    inline const SymbolStyle* GetNodeSymbolStyle(TypeId type) const
    {
      if (type<nodeSymbolStyles.size()) {
        return nodeSymbolStyles[type];
      }
      else {
        return NULL;
      }
    }

    /**
     * @brief
     *
     * @param type
     * @return IconStyle *
     */
    inline IconStyle* GetNodeIconStyle(TypeId type) const
    {
      if (type<nodeIconStyles.size()) {
        return nodeIconStyles[type];
      }
      else {
        return NULL;
      }
    }

    /**
     * @brief
     *
     * @param type
     * @return const LabelStyle *
     */
    const LabelStyle* GetNodeRefLabelStyle(TypeId type) const
    {
      if (type<nodeRefLabelStyles.size()) {
        return nodeRefLabelStyles[type];
      }
      else {
        return NULL;
      }
    }

    /**
     * @brief
     *
     * @param type
     * @return const LabelStyle *
     */
    const LabelStyle* GetNodeLabelStyle(TypeId type) const
    {
      if (type<nodeLabelStyles.size()) {
        return nodeLabelStyles[type];
      }
      else {
        return NULL;
      }
    }

    /**
     * @brief
     *
     * @param type
     * @return const LineStyle *
     */
    const LineStyle* GetWayLineStyle(TypeId type) const
    {
      if (type<wayLineStyles.size()) {
        return wayLineStyles[type];
      }
      else {
        return NULL;
      }
    }

    /**
     * @brief
     *
     * @param type
     * @return const LabelStyle *
     */
    const LabelStyle* GetWayRefLabelStyle(TypeId type) const
    {
      if (type<wayRefLabelStyles.size()) {
        return wayRefLabelStyles[type];
      }
      else {
        return NULL;
      }
    }

    /**
     * @brief
     *
     * @param type
     * @return const LabelStyle *
     */
    const LabelStyle* GetWayNameLabelStyle(TypeId type) const
    {
      if (type<wayNameLabelStyles.size()) {
        return wayNameLabelStyles[type];
      }
      else {
        return NULL;
      }
    }

    /**
     * @brief
     *
     * @param type
     * @return const FillStyle *
     */
    const FillStyle* GetAreaFillStyle(TypeId type) const
    {
      if (type<areaFillStyles.size()) {
        return areaFillStyles[type];
      }
      else {
        return NULL;
      }
    }

    /**
     * @brief
     *
     * @param type
     * @return const LabelStyle *
     */
    const LabelStyle* GetAreaLabelStyle(TypeId type) const
    {
      if (type<areaLabelStyles.size()) {
        return areaLabelStyles[type];
      }
      else {
        return NULL;
      }
    }

    /**
     * @brief
     *
     * @param type
     * @return const SymbolStyle *
     */
    const SymbolStyle* GetAreaSymbolStyle(TypeId type) const
    {
      if (type<areaSymbolStyles.size()) {
        return areaSymbolStyles[type];
      }
      else {
        return NULL;
      }
    }

    /**
     * @brief
     *
     * @param type
     * @return IconStyle *
     */
    inline IconStyle* GetAreaIconStyle(TypeId type) const
    {
      if (type<areaIconStyles.size()) {
        return areaIconStyles[type];
      }
      else {
        return NULL;
      }
    }

  };
}

#endif
