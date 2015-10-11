package org.toxiclab.droidg2;

import java.net.URL;
import java.util.ArrayList;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;

public class S2A_Adapter extends ArrayAdapter<SearchEntry2>{
	private Context m_context;
	private int m_id;
	private static SearchEntry2[] ss = null;
	private static S2A_Adapter _self = null;
	
	public S2A_Adapter(Context context, int textViewResourceId) {
		super(context, textViewResourceId);
		// TODO Auto-generated constructor stub
		m_context = context;
		m_id = textViewResourceId;
		_self = this;
	}
	
	@Override
	public int getCount(){
		if(ss == null) return 0;
		else return ss.length;
	}
	
	@Override
	public SearchEntry2 getItem(int a){
		return ss[a];
	}
	
	@Override
	public View getView(int a, View convertView, ViewGroup parent){
		View result = null;
		SearchEntry2 _s = null;
		if(convertView == null){
			//inflate the xml view
			LayoutInflater inflate = (LayoutInflater)
            		m_context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
			RelativeLayout layout = (RelativeLayout)inflate.inflate(R.layout.search_item, null);
			result = layout;
		}
		else{
			//use the view directly
			result = convertView;
		}
		
		_s = getItem(a);
		ImageView type = (ImageView)result.findViewById(R.id.ImageView01);
		TextView t1 = (TextView)result.findViewById(R.id.TextView01);
		TextView t2 = (TextView)result.findViewById(R.id.TextView02);
		TextView t3 = (TextView)result.findViewById(R.id.TextView03);
		TextView t4 = (TextView)result.findViewById(R.id.TextView04);

		t1.setText(_s.title);
		t2.setText(_s.size);
		t3.setText("");
		t4.setText(_s.source);
				
		int typex = MyHTMLParser.getType(_s.type);
		if(typex == MyHTMLParser.FILETYPE_AUDIO)
			type.setImageResource(R.drawable.music);
		else if(typex == MyHTMLParser.FILETYPE_IMAGE)
			type.setImageResource(R.drawable.picture);
		else if(typex == MyHTMLParser.FILETYPE_ZIP)
			type.setImageResource(R.drawable.pack);
		else if(typex == MyHTMLParser.FILETYPE_VIDEO)
			type.setImageResource(R.drawable.video);
		else /*if(typex == FILETYPE_DOC)*/
			type.setImageResource(R.drawable.document);
		
		return result;
	}
	
	public static void updateFromSearch2(Search_2 h){
		if(h == null) return;
		ArrayList<SearchEntry2> als = new ArrayList<SearchEntry2>();
		if(h.title == null) return;
		for(int i=0; i<h.title.size(); ++i){
			als.add(new SearchEntry2(
					h.type.get(i), h.link.get(i), h.title.get(i), h.size.get(i), h.source.get(i), h.sid.get(i), h.rid.get(i)));
		}
		ss = als.toArray(new SearchEntry2[0]);
//		if(_self != null)
//			_self.notifyDataSetChanged();
	}
	
}

class SearchEntry2{
	public String type;
	public URL link;
	public String title;
	public String size;
	public String source;
	public int searchID;
	public int resultID;
	
	public SearchEntry2(String type, URL link, String title, String size, String source, int searchID, int resultID){
		this.type = type;
		this.link = link;
		this.title = title;
		this.size = size;
		this.source = source;
		this.searchID = searchID;
		this.resultID = resultID;
	}
}