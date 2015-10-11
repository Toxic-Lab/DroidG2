package org.toxiclab.droidg2;

import java.net.URL;
import java.util.ArrayList;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.RelativeLayout;
import android.widget.TextView;

public class SA_Adapter extends  ArrayAdapter<SearchEntry> {
	private Context m_context;
	private int m_id;
	private static SearchEntry[] ss = null;
	private static SA_Adapter _self = null;
	
	public SA_Adapter(Context context, int textViewResourceId) {
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
	public SearchEntry getItem(int a){
		return ss[a];
	}
	
	@Override
	public View getView(int a, View convertView, ViewGroup parent){
		View result = null;
		SearchEntry _s = null;
		if(convertView == null){
			//inflate the xml view
			LayoutInflater inflate = (LayoutInflater)
            		m_context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
			RelativeLayout layout = (RelativeLayout)inflate.inflate(R.layout.sever_item, null);
			result = layout;
		}
		else{
			//use the view directly
			result = convertView;
		}
		
		_s = getItem(a);
		TextView t1 = (TextView)result.findViewById(R.id.textView1);
		TextView t2 = (TextView)result.findViewById(R.id.textView2);
		
		t1.setText(_s.title);
		t2.setText(String.format(SearchActivity.saResult, _s.result));
		
		return result;
	}
	
	public static void updateFromSearch(Search h){
		ArrayList<SearchEntry> als = new ArrayList<SearchEntry>();
		if(h == null) return;
		for(int i=0; i<h.title.size(); ++i){
			als.add(new SearchEntry(h.ID.get(i),
					h.title.get(i), h.result.get(i), h.rate.get(i)));
		}
		ss = als.toArray(new SearchEntry[0]);
		if(_self != null)
			_self.notifyDataSetChanged();
	}
}

class SearchEntry{
	public String title;
	public String result;
	public int id;
	public int rate;
	
	public SearchEntry(int id, String title, String result, int rate){
		this.title = title;
		this.result = result;
		this.id = id;
		this.rate = rate;
	}
}