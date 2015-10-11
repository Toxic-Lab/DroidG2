package org.toxiclab.droidg2;

import java.util.TimerTask;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.res.Resources;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ListView;
import android.widget.Toast;

public class Search_2_Activity extends Activity{
	
	private S2A_Adapter adapter = null;
	private ListView lv = null;
	private S2A r = null;
	private int ID;
	private String sort = "";
	private boolean bb = false;
	private int prevHits = 0;
	
	/* < 500 1s
	 * < 1000 1.5s
	 * < 1500 2s
	 * < 2000 3s
	 * over 2000 5s
	 */
	private int updateInterval = 0;
	private int searchEntry = 0;
	
	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.search2);
        
        adapter = new S2A_Adapter(this, R.layout.search_item);
        lv = (ListView)findViewById(R.id.listView1);
        //setTitle("Search result");
        r = new S2A();
        
        lv.setAdapter(adapter);
        
        Bundle bundle = this.getIntent().getExtras();
        String display_url= bundle.getString("ID") ;
        ID = Integer.parseInt(display_url);
        String input = bundle.getString("Result");
        int x = Integer.parseInt(input);
        updateInterval = getUpdateInterval(x);        
        
        lv.setOnItemClickListener(new OnItemClickListener(){

			@Override
			public void onItemClick(AdapterView<?> arg0, View arg1, int arg2,
					long arg3) {
				// TODO Auto-generated method stub
				//call the url
				try{
					final SearchEntry2 se = adapter.getItem((int)arg3);
					new Thread(new Runnable(){
						public void run(){
							main._self.createDownload( se.searchID, se.resultID);
						}
					}).start();

					Toast toast = Toast.makeText(Search_2_Activity.this, String.format("Download \"%s\" has been added.", se.title), Toast.LENGTH_LONG);
					toast.show();
				}
				catch(Exception ex){
					ex.printStackTrace();
				}
				
			}
        	
        });
	}
	
	public int getUpdateInterval(int nresult){
		if(nresult < 500)
			return  1000;
		else if(nresult < 1000)
			return 1500;
		else if(nresult < 1500)
			return 2000;
		else if(nresult < 2000)
			return 3000;
		else
			return 5000;
	}
	
    public void onPause(){
    	super.onPause();
    	if(r != null)
    		//main.m_handler.removeCallbacks(r);
    		r.cancel();
    }
    
    public void onResume(){
    	super.onResume();
    	r = new S2A();
    	main.m_timer.scheduleAtFixedRate(r, 1, updateInterval);
    }
    
    public void onStop(){
    	super.onStop();
    	if(r != null)
    		//main.m_handler.removeCallb acks(r);
    		r.cancel();
    }
    
    class S2A extends TimerTask{

    	@Override
    	public void run() { 
    		final Search hx = MyHTMLParser.parseSearch();
    		for(int i=0; i<hx.ID.size(); ++i){
    			if(hx.ID.get(i) == ID){
    				int a = Integer.parseInt(hx.result.get(i));
    				if(prevHits == a)
    					return; //nothing to update.
    			};
    					
    		}
    		final Search_2 h = MyHTMLParser.parseSearch_2(ID, sort, bb);
    		adapter.updateFromSearch2(h);
    		int temp = searchEntry;
    		searchEntry = h.title.size();
    		//do the update here
    		main.m_handler.post(new Runnable(){
    			public void run(){
    				//adapter.updateFromSearch2(h);
    				adapter.notifyDataSetChanged();
    			}
    		});
    		
    		if(temp <= 500 && searchEntry > 500){
    			updateInterval = 1500;
    			restartTimer();
    		}
    		else if(temp <=1000 && searchEntry > 1000){
    			updateInterval = 2000;
    			restartTimer();
    		}
    		else if(temp <= 1500 && searchEntry > 1500){
    			updateInterval = 3000;
    			restartTimer();
    		}
    		else if(temp <= 2000 && searchEntry > 2000){
    			updateInterval = 5000;
    			restartTimer();
    		}
    	}
    }     
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu){
    	MenuInflater inflater = getMenuInflater();
    	inflater.inflate(R.menu.search2_menu, menu);
    	return true;
    }
    
    @Override
    public boolean onOptionsItemSelected(MenuItem item){
    	switch(item.getItemId()){
    	case R.id.item1:
    		refresh();
    		return true;
    	case R.id.item2:
    		sort();
    		return true;
    	default:
    		return super.onContextItemSelected(item);
    	}
    }
    
    private void sort(){
    	//final String[] item = new String[] {"Name asc", "Name desc", "Size asc", "Size desc", "Number of sources asc", "Number of sources desc", "none"};
    	Resources res = getResources();
		final String[] item = res.getStringArray(R.array.sortEn);
		
    	AlertDialog.Builder builder = new AlertDialog.Builder(this);
    	builder.setTitle(getString(R.string.sort));
    	int checked = 6;
    	if(sort.compareTo(MyHTMLParser.sortby_name)==0 && bb)
    		checked = 0;
    	else if(sort.compareTo(MyHTMLParser.sortby_name) == 0 && !bb)
    		checked = 1;
    	else if(sort.compareTo(MyHTMLParser.sortby_size) == 0 && bb)
    		checked = 2;
    	else if(sort.compareTo(MyHTMLParser.sortby_size) == 0 && !bb)
    		checked = 3;
    	else if(sort.compareTo(MyHTMLParser.sortby_source) == 0 && bb)
    		checked = 4;
    	else if(sort.compareTo(MyHTMLParser.sortby_source) == 0 && !bb)
    		checked = 5;
    	builder.setSingleChoiceItems(item, checked, new DialogInterface.OnClickListener() {
			
			@Override
			public void onClick(DialogInterface dialog, int which) {
				// TODO Auto-generated method stub
				switch(which){
				case 0:
					sort = MyHTMLParser.sortby_name;
					bb = true;
					return;
				case 1:
					sort = MyHTMLParser.sortby_name;
					bb = false;
					return;
				case 2:
					sort = MyHTMLParser.sortby_size;
					bb = true;
					return;
				case 3:
					sort = MyHTMLParser.sortby_size;
					bb = false;
					return;
				case 4:
					sort = MyHTMLParser.sortby_source;
					bb = true;
					return;
				case 5:
					sort = MyHTMLParser.sortby_source;
					bb = false;
					return;
				case 6:
					sort = MyHTMLParser.sortby_none;
					bb = false;
					return;
				}
			}
		});
    	
    	AlertDialog alert = builder.create();
    	alert.show();    	
    }
    
    public void refresh(){
    	main.m_timer.schedule(new S2A(), 1);
    }
    
    public void restartTimer(){
    	r.cancel();
    	r = new S2A();
    	main.m_timer.scheduleAtFixedRate(r, 1, updateInterval);
    }
    
}
