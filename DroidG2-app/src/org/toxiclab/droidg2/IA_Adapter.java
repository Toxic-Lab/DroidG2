package org.toxiclab.droidg2;

import java.util.ArrayList;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;

public class IA_Adapter extends ArrayAdapter<IncomingFile>{
	private Context m_context;
	private int m_id;
	private static IncomingFile[] ss = null;
	private static IA_Adapter _self = null;
	
	public IA_Adapter(Context context, int textViewResourceId) {
		super(context, textViewResourceId);
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
	public IncomingFile getItem(int a){
		return ss[a];
	}
	
	@Override
	public View getView(int a, View convertView, ViewGroup parent){
		View result = null;
		IncomingFile _s = null;
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
		
		t1.setText(_s.name);
		t2.setText(_s.size);
		t3.setText("");
		t4.setText("");
		
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
	
	public static void updateFromIncoming(Incoming h){
		ArrayList<IncomingFile> als = new ArrayList<IncomingFile>();
		if(h == null) return;
		for(int i=0; i<h.name.size(); ++i){
			als.add(new IncomingFile(
					h.name.get(i), h.size.get(i), h.type.get(i), h.parent.get(i)));
		}
		ss = als.toArray(new IncomingFile[0]);
		if(_self != null)
			_self.notifyDataSetChanged();
	}	
}

class IncomingFile{
	String name;
	String size;
	String type;
	String parent;
	
	public IncomingFile(String name, String size, String type, String parent){
		this.name = name;
		this.size = size;
		this.type = type;
		this.parent = parent;
	}
}
