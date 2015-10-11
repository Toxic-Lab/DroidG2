package org.toxiclab.droidg2;

import java.util.ArrayList;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.RelativeLayout;
import android.widget.TextView;

public class CA_Adapter extends ArrayAdapter<Server> {
	private Context m_context;
	private int m_id;
	private static Server[] ss = null;
	private static Object ssl = new Object();
	private static CA_Adapter _self = null;
	
	public CA_Adapter(Context context, int textViewResourceId) {
		super(context, textViewResourceId);
		// TODO Auto-generated constructor stub
		m_context = context;
		m_id = textViewResourceId;
		_self = this;
	}
	
	@Override
	public int getCount(){
		synchronized(ssl){
			if(ss == null) 
				return 0;
			return ss.length;
		}
	}
	
	@Override
	public Server getItem(int a){
		synchronized(ssl){
			return ss[a];
		}
	}
	
	@Override
	public View getView(int a, View convertView, ViewGroup parent){
		View result = null;
		Server _s = null;
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
		
		t1.setText(_s.name);
		String st2 = null;
		if(_s.status.compareTo("connecting") == 0)
			st2 = ConnectionActivity._connecting;
		else if(_s.status.compareTo("connected") == 0)
			st2 = ConnectionActivity._connected;
		else if(_s.status.compareTo("handshaking") == 0)
			st2 = ConnectionActivity._handshaking;
		t2.setText(st2);
		
		return result;
	}
	
	public static void updateFromHome(Home h){
		if(h == null) return;
		synchronized(ssl){
			ArrayList<Server> als = new ArrayList<Server>();
			for(int i=0; i<h.server.size(); ++i){
				als.add(new Server(h.server.get(i), h.status.get(i)));
			}
			ss = als.toArray(new Server[0]);
			if(_self != null)
				_self.notifyDataSetChanged();
		}
	}
}

class Server{
	String name;
	String status;
	
	public Server(String a, String b){
		name = a;
		status = b;
	}
}