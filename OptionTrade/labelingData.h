#pragma once
#ifndef LABELING_DATA_H
#define LABELING_DATA_H

#include "ShapeData.H"

struct LabelingDataUnit
{
	shapeData::eShapeType shape_type;
	QString label;
	shapeData::PointsType points;
};

struct SingleLabelingData
{
	void addData(LabelingDataUnit unit) { single_data.push_back(unit); }
	std::vector<LabelingDataUnit> single_data;
};

struct GeneralLabelingData
{
	//size为1时，兼容通用图片格式的json
	//大于1时，以array方式保存为json，这就是jmg格式保存的图片数据
	std::vector<SingleLabelingData> general_data;
};
#endif // !LABELING_DATA_H
