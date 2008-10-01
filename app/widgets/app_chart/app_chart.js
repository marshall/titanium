
/*
 * Copyright 2006-2008 Appcelerator, Inc.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *    http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. 
 */



Appcelerator.Widget.Chart =
{
	flashRequired: true,
	flashVersion: 8.0,
	
	getName: function()
	{
		return 'appcelerator chart';
	},
	getDescription: function()
	{
		return 'chart widget';
	},
	getVersion: function()
	{
		return '1.1.1';
	},
	getSpecVersion: function()
	{
		return 1.0;
	},	
	getAuthor: function()
	{
		return 'Amro Mousa';
	},
	getModuleURL: function ()
	{
		return 'http://www.appcelerator.org';
	},
	isWidget: function ()
	{
		return true;
	},
	getWidgetName: function()
	{
		return 'app:chart';
	},
	getActions: function()
	{
		return ['execute','update'];
	},	
	getAttributes: function()
	{
		var T = Appcelerator.Types;
		return [{name: 'on', optional: false, description: "Used to display the chart.",
		         type: T.onExpr},
		         
				{name: 'type', optional: false, defaultValue: '',
				 description: "enumerated value, or property on payload to find value",
				 type: T.openEnumeration('bar', 'pie', 'line')},
				 
				{name: 'title', optional: true, defaultValue: '', description: "Title displayed at the top of the chart."},
				{name: 'color', optional: true, defaultValue: '#477398', 
				    description: "Specifies the base color of the chart or the color for each bar/slice/line ." + 
				        " You can provide a single hex color or csv color list"},
				    
				{name: 'angle', optional: true, defaultValue: '15', type: T.number, description: 
				        "Angle for '3D' graphs, bar and pie (in degrees)"},
				{name: 'thickness', optional: true, defaultValue: '15', type: T.number, description: "Thickness of '3D' graphs (in pixels)"},
				{name: 'width', optional: true, defaultValue: '400', type: T.number, description: "The width of the chart (in pixels)"},
				{name: 'height', optional: true, defaultValue: '360', type: T.number, description: "The height of the chart (in pixels)"},
				{name: 'chartMode', optional: true, defaultValue: 'clustered',
				 description: "sub type for bar and line charts",
				 type: T.openEnumeration('line', 'clustered', 'stacked', '100% stacked')},
				{name: 'barOrientation', optional: true, defaultValue: '400', type: T.number, description: '"vertical" or "horizontal" orientation for a bar chart'},
				{name: 'rotateXAxisLabel', optional: true, defaultValue: 'false', description: '"true" or "false". If "true" x axis labels are rotated by 90 degrees.'},
				{name: 'rotateYAxisLabel', optional: true, defaultValue: 'false', description: '"true" or "false". If "true" y axis labels are rotated by 90 degrees.'},
				{name: 'legend', optional: true, defaultValue: 'false', type: T.bool, description: '"true" or "false". If "true" a legend will be displayed.'},
				{name: 'brightness_step', optional: true, defaultValue: '15', type: T.number, description: 'How much the brightness of the chart increases'},
				{name: 'textSize', optional: true, defaultValue: '11', type: T.number, description: 'Font size (in pixels)'},
				{name: 'property', optional: false, defaultValue: '', type: T.identifier, description: 'The property of the message to use to populate the chart'},
				{name: 'chartTitles', optional: true, defaultValue: '', description: 'Array property name that contains the titles for the legend and labels composite bar charts.'},
				{name: 'fillAlpha', optional: true, defaultValue: '30', type: T.number, description: "Specifies the opacity of the shaded area under a line in a line chart (percentage)"},
				{name: 'indicator', optional: true, defaultValue: 'false', type: T.bool, description: "Enable a mouseover bubble to show the value of every line at a given x value on a line graph"},
				{name: 'marginTop', optional: true, defaultValue: '80', type: T.number, description: "Specifies the space between the top of the chart and the start of the line or bar chart (pixels)"},
				{name: 'marginLeft', optional: true, defaultValue: '50', type: T.number, description: "Specifies the space between the left of the chart and the start of the line or bar chart (pixels)"},
				{name: 'marginRight', optional: true, defaultValue: '50', type: T.number, description: "Specifies the space between the right of the chart and the start of the line or bar chart (pixels)"},
				{name: 'marginBottom', optional: true, defaultValue: '50', type: T.number, description: "Specifies the space between the bottom of the chart and the start of the line or bar chart (pixels)"},
				{name: 'legendHighlight', optional: true, defaultValue: 'true', type: T.bool, description: "Specifies whether the lines on the line chart will be 'highlighted' when one mouses over a legend value for that line."},
				{name: 'backgroundColor', optional: true, defaultValue: '#FFFFFF'},
				{name: 'radius', optional: true, defaultValue: '90', type: T.number},
				{name: 'precision', optional: true, defaultValue: '2', type: T.number},
				{name: 'innerRadius', optional: true, defaultValue: '30', type: T.number, description: "Specifies the radius of the 'hole' in the center of the pie chart (pixels)"},
				{name: 'animation', optional: true, defaultValue: 'true', type: T.bool},
				{name: 'oneBalloon', optional: true, defaultValue: 'false', type: T.bool, description: "Specifies whether an indicator balloon will be shown for all lines or only the line closest to the mouse pointer on the line graph. Possible values are 'true' or 'false'."},
				{name: 'xAxisLabel', optional: true, defaultValue: ''},
				{name: 'xAxisYPosition', optional: true, defaultValue: '0', type: T.number},
				{name: 'xAxisXPosition', optional: true, defaultValue: '0', type: T.number},
				{name: 'yAxisLabel', optional: true, defaultValue: ''},
				{name: 'yAxisYPosition', optional: true, defaultValue: '0', type: T.number},
				{name: 'yAxisXPosition', optional: true, defaultValue: '0', type: T.number},
				{name: 'dataValueMax', optional: true, defaultValue: '0', type: T.number}];
	},
	buildWidget: function(element, parameters)
	{
		var html = '<div id="' + element.id + '"></div>';
		
		return {
			'presentation' : html,
			'position' : Appcelerator.Compiler.POSITION_REPLACE,
			'parameters': parameters
		};
	},
	getDataAsCSV: function (array_of_data)
	{
		var data_as_csv = '';
		for (var i = 0, len = array_of_data.length; i < len; i++)
		{
			data_as_csv += array_of_data[i]['name'] + ',' + array_of_data[i]['value'] + '\n';
		}
		return data_as_csv;
	},
	update: function(id, parameterMap, data)
	{
		var propertyName = parameterMap['property'];
		if (propertyName)
		{
			var array = Object.getNestedProperty(data,propertyName) || [];
			parameterMap['data'] = array;
			var update_data_as_csv = Appcelerator.Widget.Chart.getDataAsCSV(array);
			
			try
			{
				$('chart_'+id).setData(update_data_as_csv);
			} catch (exxd) 
			{
				Logger.info('Failed to set chart data for id: ' + id);
			}
		}
	},
	execute: function(id,parameterMap,data)
	{
		var type = parameterMap['type'];
		var title = parameterMap['title'];
		var color = parameterMap['color'];
		var angle = parameterMap['angle'];
		var thickness = parameterMap['thickness'];
		var width = parameterMap['width'];
		var height = parameterMap['height'];
		var chartMode = parameterMap['chartMode'];
		var barOrientation = parameterMap['barOrientation'];
		var rotateXAxisLabel = parameterMap['rotateXAxisLabel'];
		var rotateYAxisLabel = parameterMap['rotateYAxisLabel'];
		var legend_enabled = parameterMap['legend'];
		var brightness_step = parameterMap['brightness_step'];
		var propertyName = parameterMap['property'];
		var titlePropertyName = parameterMap['chartTitles'];
	 	var textSize = parameterMap['textSize'];
	  	var fillAlpha = parameterMap['fillAlpha'];
	  	var indicator = parameterMap['indicator'];
		var marginTop = parameterMap['marginTop'];
		var marginLeft = parameterMap['marginLeft'];
		var marginRight = parameterMap['marginRight'];
		var marginBottom = parameterMap['marginBottom'];
	  	var legendHighlight = parameterMap['legendHighlight'];
	  	var oneBalloon = parameterMap['oneBalloon'];
		var backgroundColor = parameterMap['backgroundColor'];
		var innerRadius = parameterMap['innerRadius'];
		var radius = parameterMap['radius'];
		var precision = parameterMap['precision'];
		var yAxisLabel = parameterMap['yAxisLabel'];
		var xAxisLabel = parameterMap['xAxisLabel'];
		var yAxisXPosition = parameterMap['yAxisXPosition'];
		var yAxisYPosition = parameterMap['yAxisYPosition'];
		var xAxisXPosition = parameterMap['xAxisXPosition'];
		var xAxisYPosition = parameterMap['xAxisYPosition'];
		var dataValueMax = parameterMap['dataValueMax'];
		var animation = parameterMap['animation'];
		
		if (animation != 'false' && type.toLowerCase() == 'pie') animation = 2;
		else if (animation != 'false' && type.toLowerCase() == 'bar') animation = 1;
		else animation = 0;
		
		if (dataValueMax != '')
		{
			dataValueMax = Object.getNestedProperty(data,dataValueMax) || '';							
		}
		
		if (barOrientation.toLowerCase() != "vertical" && barOrientation.toLowerCase() != "horizontal")
		{
			barOrientation = Object.getNestedProperty(data,barOrientation) || 'vertical';				
		}
		
		if (barOrientation.toLowerCase() == 'horizontal')
		{
			barOrientation = 'bar';
		} else
		{
			barOrientation = 'column';
		}
	
		if (rotateXAxisLabel.toLowerCase() == 'true')
		{
			rotateXAxisLabel = 90;
		} else
		{
			rotateXAxisLabel = 0;
		}
	
		if (rotateYAxisLabel.toLowerCase() == 'true')
		{
			rotateYAxisLabel = 90;
		} else
		{
			rotateYAxisLabel = 0;
		}

		if (propertyName)
		{
			array = Object.getNestedProperty(data,propertyName) || [];
			parameterMap['data'] = array;
		}
		else
		{
			array = [];
		}

		if (title != '')
		{
			var t_title = Object.getNestedProperty(data,title);
			
			if (t_title == null)
			{
				t_title = '*[No Title]*';
			}
			
			if (t_title != '*[No Title]*')
			{
				title = t_title;
			}
		}
		
		if (yAxisLabel != '')
		{
			var t_yAxisLabel = Object.getNestedProperty(data,yAxisLabel);
			
			if (t_yAxisLabel == null)
			{
				t_yAxisLabel = '*[No yAxisLabel]*';
			}
			
			if (t_yAxisLabel != '*[No yAxisLabel]*')
			{
				yAxisLabel = t_yAxisLabel;
			} else yAxisLabel = '';
		} 
		
		if (xAxisLabel != '')
		{
			var t_xAxisLabel = Object.getNestedProperty(data,xAxisLabel);
			
			if (t_xAxisLabel == null)
			{
				t_xAxisLabel = '*[No xAxisLabel]*';
			}
			
			if (t_xAxisLabel != '*[No xAxisLabel]*')
			{
				xAxisLabel = t_xAxisLabel;
			} else xAxisLabel = '';
		} 

		var data_as_csv = Appcelerator.Widget.Chart.getDataAsCSV(array);
		
		if (type.toLowerCase() != "bar" && type.toLowerCase() != "pie" && type.toLowerCase() != "line" && type.toLowerCase() != "")
		{
			type = Object.getNestedProperty(data,type,'');
		}
		
		if (type.toLowerCase() == "bar" && (chartMode.toLowerCase() != "clustered" && chartMode.toLowerCase() != "stacked" && chartMode.toLowerCase() != "100% stacked"))
		{
			chartMode = Object.getNestedProperty(data,chartMode,'clustered');
		}

		if (type.toLowerCase() == "line" && (chartMode.toLowerCase() != "line" && chartMode.toLowerCase() != "stacked" && chartMode.toLowerCase() != "100% stacked"))
		{
			chartMode = Object.getNestedProperty(data,chartMode,'line');
		}
	
		if (legend_enabled.toLowerCase() != "true" && legend_enabled.toLowerCase() != "false")
		{
			legend_enabled = Object.getNestedProperty(data,legend_enabled,'false');				
		}
			
		var color_list = color.split(',');
		if (color_list.length > 1 && type.toLowerCase() == "pie")
		{
			color_list = color;
			color = '';
			brightness_step = '';
		} else if (color_list.length == 1 && type.toLowerCase() == "pie")
		{
			color_list = '';
		}
		
		if (color_list.length > 1 && type.toLowerCase() == "bar")
		{
			color = color_list[0];
		}
		
		var stacked_balloon = '';
		if (type.toLowerCase() == "bar" || type.toLowerCase() == "line")
		{			
			titleArray = Object.getNestedProperty(data,titlePropertyName) || [];
			parameterMap['titleArray'] = titleArray;
	
			var column_titles_template = '';
			if (type.toLowerCase() == "bar")
			{
				column_titles_template = '<graph gid="{COL_NUM}"><type>column</type><title>{COL_TITLE}</title><color>{COL_COLOR}</color><alpha></alpha><data_labels><![CDATA[]]></data_labels><fill_alpha>' + fillAlpha + '</fill_alpha></graph>';
			} else if (type.toLowerCase() == "line")
			{
				column_titles_template = '<graph><axis>left</axis><title>{COL_TITLE}</title><color>{COL_COLOR}</color><color_hover>FF0000</color_hover><fill_alpha>' + fillAlpha + '</fill_alpha><balloon_text><![CDATA[{COL_TITLE} {value}]]></balloon_text></graph>';				
			}
			var chartTitlesTemp = '';
			var chartTitles = '';
			for (var i = 0, len = titleArray.length; i < len; i++)
			{
				chartTitlesTemp += column_titles_template.gsub('{COL_NUM}', i.toString()).gsub('{COL_TITLE}', titleArray[i]['title']).gsub('{COL_COLOR}', color_list[i]||'');
			}
	
			if (titleArray.length == 0)
			{
				if (type.toLowerCase() == "bar")
				{
					chartTitles = '<graph gid="0"><type>column</type><title></title><color>' + color + '</color><alpha></alpha><data_labels><![CDATA[]]></data_labels><fill_alpha>' + fillAlpha + '</fill_alpha><width></width><bullet></bullet><bullet_size></bullet_size><bullet_color></bullet_color><gradient_fill_colors></gradient_fill_colors></graph>';
				} else if (type.toLowerCase() == "line")
				{
					for (var i = 0; i < color_list.length; i++)
					{
						 chartTitles += '<graph><axis>left</axis><title></title><color>' + color_list[i] + '</color><color_hover>#FF0000</color_hover><fill_alpha>' + fillAlpha + '</fill_alpha><balloon_text><![CDATA[]]></balloon_text></graph>';
					}
				} else
				{
					chartTitles = '<graph><axis>left</axis><title></title><color>' + color + '</color><color_hover>#FF0000</color_hover><fill_alpha>' + fillAlpha + '</fill_alpha><balloon_text><![CDATA[]]></balloon_text></graph>';					
				}
			} else
			{
				chartTitles = chartTitlesTemp;
			}		
			
			var stacked_baloon_fix = (titleArray.length > 0) ? '{title}' : '{series}';
			if (chartMode.toLowerCase() == 'stacked')
			{
				stacked_balloon = '<balloon_text><![CDATA[' + stacked_baloon_fix + ': {value}]]></balloon_text>';
			} else if (chartMode.toLowerCase() == '100% stacked')
			{
				stacked_balloon = '<balloon_text><![CDATA[' + stacked_baloon_fix + ': {percents}%]]></balloon_text>';
			} else if (chartMode.toLowerCase() == 'clustered')
			{
				stacked_balloon = '<balloon_text><![CDATA[' + stacked_baloon_fix + ': {value}]]></balloon_text>';
			} else if (chartMode.toLowerCase() == 'line')
			{
				stacked_balloon = '<balloon_text><![CDATA[' + stacked_baloon_fix + ': {value}]]></balloon_text>';				
			}
		}
		
		var legend_comp = legend_enabled ? Math.floor(height*0.06) : 0;
		var g_height = height - legend_comp;
								
		var so;
		if (type.toLowerCase() == "pie")
		{
			so = new SWFObject(Appcelerator.DocumentPath + "swf/ampie.swf", "chart_" + id, width, height, "8", "#FFFFFF");
			so.addVariable("chart_settings", escape('<settings><width/><height>' + g_height + '</height><data_type>csv</data_type><csv_separator>,</csv_separator><skip_rows>0</skip_rows><font>Arial</font><text_size>' + textSize + '</text_size><text_color>#000000</text_color><decimals_separator>.</decimals_separator><thousands_separator>,</thousands_separator><redraw/><reload_data_interval/><add_time_stamp>false</add_time_stamp><precision>'+precision+'</precision><export_image_file/><exclude_invisible/><pie><x/><y></y><radius>'+radius+'</radius><inner_radius>' + innerRadius + '</inner_radius><height>' + thickness + '</height><angle>' + angle + '</angle><outline_color/><outline_alpha/><!-- main color --><base_color>' + color + '</base_color><brightness_step>' + brightness_step + '</brightness_step><colors>' + color_list +'</colors><link_target/><alpha/></pie><animation><start_time>'+animation+'</start_time><start_effect>strong</start_effect><start_radius/><start_alpha>0</start_alpha><pull_out_on_click/><pull_out_time>1.5</pull_out_time><pull_out_effect>Bounce</pull_out_effect><pull_out_radius/><pull_out_only_one/></animation><data_labels><radius/><text_color/><text_size/><max_width>140</max_width><show><![CDATA[{title}: {percents}%]]></show><show_lines/><line_color/><line_alpha/><hide_labels_percent>3</hide_labels_percent></data_labels><group><percent/><color/><title/><url/><description/><pull_out/></group><background><color>' + backgroundColor + '</color><alpha>100</alpha><border_color/><border_alpha/><file/></background><balloon><enabled/><color/><alpha>80</alpha><text_color/><text_size/><show><![CDATA[{title}: {percents}%]]></show></balloon><legend><enabled>' + legend_enabled + '</enabled><x/><y></y><width/><color>#FFFFFF</color><max_columns/><alpha>0</alpha><border_color>#000000</border_color><border_alpha>0</border_alpha><text_color/><text_size/><spacing>5</spacing><margins> <left>'+marginLeft+'</left> <top>'+marginTop+'</top> <right>'+marginRight+'</right> <bottom>'+marginBottom+'</bottom> </margins><key><size>16</size><border_color/></key></legend><strings><no_data/><export_as_image/><collecting_data/></strings><labels><label><x>0</x><y>10</y><rotate>false</rotate><width/><align>center</align><text_color/><text_size>' + textSize + '</text_size><text>' + title + '</text></label></labels></settings>'));
		} else if (type.toLowerCase() == "bar")
		{
			so = new SWFObject(Appcelerator.DocumentPath + "swf/amcolumn.swf", "chart_" + id, width, height, "8", "#FFFFFF");
			so.addVariable("chart_settings", escape('<settings><type>' + barOrientation + '</type><width></width><height>' + g_height + '</height><data_type>csv</data_type><csv_separator>,</csv_separator><skip_rows>0</skip_rows><font>Arial</font><text_size>' + textSize + '</text_size><text_color>#000000</text_color><decimals_separator>.</decimals_separator><thousands_separator>,</thousands_separator><redraw>false</redraw><reload_data_interval>0</reload_data_interval><add_time_stamp>false</add_time_stamp><precision>2</precision><depth>' + thickness + '</depth><angle>'+angle+'</angle><export_image_file></export_image_file><column><type>' + chartMode + '</type><width>60</width><spacing>1</spacing><grow_time>'+animation+'</grow_time><grow_effect>strong</grow_effect><alpha>100</alpha><border_color>#FFFFFF</border_color><border_alpha></border_alpha><data_labels><![CDATA[]]></data_labels><data_labels_text_color></data_labels_text_color><data_labels_text_size></data_labels_text_size><data_labels_position></data_labels_position>' + stacked_balloon + '<link_target></link_target><gradient></gradient></column><line><connect></connect><width></width><alpha></alpha><fill_alpha>' + fillAlpha + '</fill_alpha><bullet></bullet><bullet_size></bullet_size><data_labels><![CDATA[{value}]]></data_labels><data_labels_text_color></data_labels_text_color><data_labels_text_size></data_labels_text_size><balloon_text><![CDATA[{value}]]></balloon_text><link_target></link_target></line><background><color>' + backgroundColor + '</color><alpha>100</alpha><border_color></border_color><border_alpha></border_alpha><file></file></background><plot_area><color></color><alpha></alpha><margins><left>'+marginLeft+'</left><top>'+marginTop+'</top><right>'+marginRight+'</right><bottom>'+marginBottom+'</bottom></margins></plot_area><grid><category><color>#000000</color><alpha>5</alpha><dashed>true</dashed><dash_length>5</dash_length></category><value><color>#000000</color><alpha>5</alpha><dashed>false</dashed><dash_length>5</dash_length><approx_count>10</approx_count></value></grid><values><category><enabled>true</enabled><frequency>1</frequency><rotate>' + rotateXAxisLabel + '</rotate><color></color><text_size>' + textSize + '</text_size></category><value><enabled>true</enabled><min>0</min><max>'+dataValueMax+'</max><frequency>1</frequency><rotate>' + rotateYAxisLabel + '</rotate><skip_first>true</skip_first><skip_last>false</skip_last><color></color><text_size>' + textSize + '</text_size><unit></unit><unit_position>right</unit_position><integers_only></integers_only></value></values><axes><category><color>#000000</color><alpha>100</alpha><width>1</width><tick_length>7</tick_length></category><value><color>#000000</color><alpha>100</alpha><width>1</width><tick_length>7</tick_length></value></axes><balloon><enabled>true</enabled><color></color><alpha>80</alpha><text_color></text_color><text_size>' + textSize + '</text_size></balloon><legend><enabled>' + legend_enabled + '</enabled><x></x><y>' + Math.floor(height*0.1) + '</y><width></width><color>#FFFFFF</color><alpha>0</alpha><border_color>#000000</border_color><border_alpha>0</border_alpha><text_color></text_color><text_size>' + textSize + '</text_size><spacing>5</spacing><margins>0</margins><key><size>16</size><border_color></border_color></key></legend><strings><no_data></no_data><export_as_image></export_as_image><collecting_data></collecting_data></strings><labels><label><x>0</x><y>5</y><rotate></rotate><width></width><align>center</align><text_color></text_color><text_size>' + textSize + '</text_size><text><![CDATA[' + title + ']]></text></label>  <label><x>'+ yAxisXPosition +'</x><y>' + yAxisYPosition + '</y><rotate>true</rotate><width></width><align>center</align><text_color></text_color><text_size>' + textSize + '</text_size><text><![CDATA[' + yAxisLabel + ']]></text></label><label><x>'+xAxisXPosition+'</x><y>'+xAxisYPosition+'</y><rotate>false</rotate><width></width><align>center</align><text_color></text_color><text_size>' + textSize + '</text_size><text><![CDATA[' + xAxisLabel + ']]></text></label>  </labels><graphs>' + chartTitles + '</graphs></settings>'));		
		} else if (type.toLowerCase() == "line")
		{
			so = new SWFObject(Appcelerator.DocumentPath + "swf/amline.swf", "chart_" + id, width, height, "8", "#FFFFFF");
			so.addVariable("chart_settings", escape('<settings> <width></width><height>' + g_height + '</height><data_type>csv</data_type> <type>' + chartMode + '</type> <csv_separator>,</csv_separator> <skip_rows></skip_rows> <font></font> <text_size>' + textSize + '</text_size> <text_color></text_color> <decimals_separator>.</decimals_separator> <thousands_separator>,</thousands_separator> <redraw></redraw><reload_data_interval></reload_data_interval> <add_time_stamp></add_time_stamp><connect></connect> <hide_bullets_count></hide_bullets_count> <export_image_file></export_image_file><link_target></link_target><background> <color>' + backgroundColor + '</color> <alpha>100</alpha> <border_color></border_color> <border_alpha></border_alpha> <file></file></background><plot_area> <color></color> <alpha></alpha> <border_color></border_color> <border_alpha></border_alpha> <margins> <left>'+marginLeft+'</left> <top>'+marginTop+'</top> <right>'+marginRight+'</right> <bottom>'+marginBottom+'</bottom> </margins> </plot_area><scroller> <enabled></enabled> <y></y> <color></color> <alpha></alpha> <bg_color></bg_color> <bg_alpha></bg_alpha> <height></height> </scroller><grid> <x> <enabled></enabled> <color></color><alpha>5</alpha><dashed>true</dashed><dash_length>5</dash_length> <approx_count>10</approx_count> </x> <y_left> <enabled></enabled> <color></color> <alpha>5</alpha><dashed></dashed><dash_length></dash_length> <approx_count></approx_count> </y_left> <y_right> <enabled></enabled> <color></color> <alpha>5</alpha><dashed></dashed><dash_length></dash_length> <approx_count></approx_count> </y_right> </grid><values> <x> <enabled>true</enabled> <rotate>' + rotateXAxisLabel + '</rotate> <frequency></frequency> <skip_first></skip_first> <skip_last></skip_last> <color></color> <text_size>' + textSize + '</text_size> </x> <y_left> <enabled>true</enabled> <rotate>' + rotateYAxisLabel + '</rotate> <min></min> <max></max> <strict_min_max></strict_min_max> <frequency></frequency> <skip_first></skip_first> <skip_last></skip_last> <color></color> <text_size>' + textSize + '</text_size> <unit></unit> <unit_position></unit_position> <integers_only></integers_only> </y_left> <y_right> <enabled></enabled> <rotate>' + rotateYAxisLabel + '</rotate> <min></min> <max></max> <strict_min_max></strict_min_max> <frequency></frequency> <skip_first></skip_first> <skip_last></skip_last> <color></color> <text_size>' + textSize + '</text_size> <unit></unit> <unit_position></unit_position> <integers_only></integers_only> </y_right> </values><axes> <x> <color></color> <alpha></alpha> <width></width> <tick_length></tick_length> </x> <y_left> <color></color> <alpha></alpha> <width></width> <tick_length></tick_length> </y_left> <y_right> <color></color> <alpha></alpha> <width></width> <tick_length></tick_length> </y_right> </axes><indicator> <enabled>' + indicator + '</enabled> <zoomable></zoomable> <color></color> <line_alpha></line_alpha> <selection_color></selection_color> <selection_alpha></selection_alpha> <x_balloon_enabled></x_balloon_enabled> <x_balloon_text_color></x_balloon_text_color> <y_balloon_text_size>' + textSize + '</y_balloon_text_size> <y_balloon_on_off>'+legendHighlight+'</y_balloon_on_off> <one_y_balloon>'+oneBalloon+'</one_y_balloon> </indicator><legend><enabled>' + legend_enabled + '</enabled> <x></x> <y>' + Math.floor(height*0.1) + '</y> <width></width> <max_columns></max_columns> <color></color> <alpha></alpha> <border_color></border_color> <border_alpha></border_alpha> <text_color></text_color> <text_color_hover></text_color_hover> <text_size>' + textSize + '</text_size> <spacing>5</spacing> ' +
					'<margins></margins> <graph_on_off>false</graph_on_off> <key> <size></size> <border_color></border_color> <key_mark_color></key_mark_color> </key> <values> <enabled></enabled> <width></width> <align></align> <text><![CDATA[]]></text> </values> </legend><zoom_out_button> <x></x> <y></y> <color></color> <alpha></alpha> <text_color></text_color> <text_color_hover></text_color_hover> <text_size>' + textSize + '</text_size> <text></text> </zoom_out_button><help> <button> <x></x> <y></y> <color></color> <alpha></alpha> <text_color></text_color> <text_color_hover></text_color_hover> <text_size>' + textSize + '</text_size> <text></text> </button> <balloon> <color></color> <alpha></alpha> <width></width> <text_color></text_color> <text_size>' + textSize + '</text_size> <text><![CDATA[]]></text> </balloon> </help><strings> <no_data></no_data> <export_as_image></export_as_image> <error_in_data_file></error_in_data_file> <collecting_data></collecting_data> <wrong_zoom_value></wrong_zoom_value> </strings><labels><label> <x></x> <y>5</y> <rotate></rotate> <width></width> <align>center</align> <text_color></text_color> <text_size>' + textSize + '</text_size> <text> <![CDATA[' + title + ']]> </text> </label></labels> <graphs>' + chartTitles + '</graphs></settings>'));
		} else
		{
			throw ('Appcelerator Chart Error: type must be set to either "pie", "bar", "line" or a valid value passed via the incoming property message; "' + type + '" is not a valid chart type.');
		}
		
		if (so != null)
		{
			so.addVariable("path", Appcelerator.DocumentPath + "swf/");		
			so.addVariable("chart_data", escape(data_as_csv));
			so.addVariable("preloader_color", "#FFFFFF");
			so.write(id);
		}			
	}
};

Appcelerator.Widget.registerWithJS('app:chart',Appcelerator.Widget.Chart,['swfobject.js']);
