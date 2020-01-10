#pragma once
#ifndef SHAPE_H
#define SHAPE_H

#include <QString>
#include <QPoint>
#include <vector>
#include <QPainter>
#include <QColor>

class shapeData
{
public:
	using PointsType = std::vector<QPoint>;
	enum eShapeType
	{
		rectangle,
		ploygon
	};

	shapeData();
	shapeData(eShapeType shape_type, QString label);
	~shapeData();
	
	bool IsVisibel() { return visibel; }
	void setVisibel(bool visibel_) { visibel = visibel_; }

	bool IsFill() { return fill; }
	void setFill(bool fill_) { fill = fill_; }

	bool IsSelected() { return selected; }
	void setSelected(bool selected_) { selected = selected_; }

	bool IsPloygonShutting() { return bIsPloygonShutting; }
	void setPloygonShutting(bool shutting) { bIsPloygonShutting = shutting; }

	int getIndex() { return index; }
	int getIndex() const{ return index; }
	void setIndex(int index_) { index = index_; }

	eShapeType getShapeType() { return shape_type; }
	void setShapeType(eShapeType shape_type_) {shape_type = shape_type_;}

	QString getLabel() { return label; }
	QString getLabel() const { return label; }
	void setLabel(QString label_) { label = label_; }

	void addPoint(QPoint point_) { points.push_back(point_); }
	void setPoints(PointsType points_) { points = points_; }
	
	PointsType getPoints() { return points; }
	void clearPoints() { points.clear(); }

	bool IsRectangle() { return shape_type == rectangle; }
	bool IsPloygon() { return shape_type == ploygon; }

	void setHighlightPointIndex(int point_highlight_index_) { point_highlight_index = point_highlight_index_; }
	QColor getVertexFillColor() { return vertex_fill_color; }

	bool operator==(shapeData shape_unit)const { return label == shape_unit.getLabel() && points == shape_unit.getPoints(); }
	void drawVertex(QPainterPath &path, int point_index, float fScale);

	//selected_vertex_index:选中顶点在points中的index，默认值-1表示未选中顶点
	//包含labelme中的moveBy/moveVertexBy功能
	void moveBy(QPoint offset, int selected_vertex_index = -1);

	void clearData();

private:
	bool visibel;         //label列表中是否被选中
	bool fill;
	bool selected;
	bool bIsPloygonShutting;
	int index;
	eShapeType shape_type;

	QString label;
	PointsType points;

	int point_highlight_index;
	QColor vertex_fill_color;

};

//shape类的比较函数
struct shape_comp
{
    bool operator()(const shapeData &s1, const shapeData &s2)
    {
    	return (s1.getIndex() < s2.getIndex());
    }
};

#endif // !SHAPE_H
