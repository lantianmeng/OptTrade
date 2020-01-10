#include "stdafx.h"
#include "ShapeData.h"

shapeData::shapeData()
{
	shape_type = eShapeType::ploygon;
	label = "";
	visibel = true;
	fill = false;
	selected = false;
	bIsPloygonShutting = false;
	index = -1;
	point_highlight_index = -1;
}

shapeData::shapeData(eShapeType shape_type_, QString label_):shape_type(shape_type_), label(label_)
{
	visibel = true;
	fill = false;
	selected = false;
	bIsPloygonShutting = false;
	index = -1;
	point_highlight_index = -1;
}

shapeData::~shapeData()
{

}

void shapeData::drawVertex(QPainterPath &path, int point_index, float fScale)
{
	QPoint point = points[point_index]*fScale;
	qreal d = 8;//point_size
	if (point_highlight_index == point_index)
	{
		path.addRect(point.x() - d / 2, point.y() - d / 2, d, d);
	}
	else
	{
		path.addEllipse(point, d / 2.0, d / 2.0);
	}

	vertex_fill_color = QColor(Qt::green);
	if (point_highlight_index >= 0)  //存在highlight point
	{
		vertex_fill_color = QColor(Qt::red);
	}
}

void shapeData::moveBy(QPoint offset, int selected_vertex_index)
{
	if (selected_vertex_index >= 0 && selected_vertex_index < points.size()) //选中的是顶点，则只移动一个点
	{
		points[selected_vertex_index] += offset;
	}
	else
	{
		//选中的不是顶点，则移动框中的所有点
		std::for_each(points.begin(), points.end(), [=](QPoint & pt)
		{
			pt += offset;
		});
	}

}

void shapeData::clearData()
{
	setFill(false);
	setSelected(false);
	setPloygonShutting(false);
	setIndex(-1);
	setHighlightPointIndex(-1);
	clearPoints();
}
