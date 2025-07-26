import streamlit as st
import firebase_admin
from firebase_admin import credentials, db
import pandas as pd
import time
from datetime import datetime

# ---------------------------
# Firebase Configuration
# ---------------------------

FIREBASE_CERT_FILE = "/path/to/your/firebase-adminsdk.json"
DATABASE_URL = "https://your-project-id.firebaseio.com/"

if not firebase_admin._apps:
    cred = credentials.Certificate(FIREBASE_CERT_FILE)
    firebase_admin.initialize_app(cred, {'databaseURL': DATABASE_URL})

# Firebase references
logs_ref = db.reference("/rfidLogs")

# ---------------------------
# RFID Tag to Item Mapping and Pricing
# ---------------------------
ITEM_MAP = {
    "5000371CDAA1": "Lays",
    "50002911EB83": "Apple",
    "50002E1598F3": "Biscuit",
    "50002B9CAE49": "Ice cream",
    "50002DEDD646": "Milk"
}

ITEM_PRICES = {
    "Lays": 50,
    "Apple": 30,
    "Biscuit": 20,
    "Ice cream": 100,
    "Milk": 60
}

REVERSE_ITEM_MAP = {v: k for k, v in ITEM_MAP.items()}

st.set_page_config(page_title="RFID Item Counter", layout="wide")
refresh_interval = 2  # seconds

# ---------------------------
# Fetch Data Function
# ---------------------------
def fetch_data():
    logs_data = logs_ref.get() or {}
    items = []
    keys_to_items = {}

    for key, entry in logs_data.items():
        tag = entry.get("tag")
        qty = entry.get("qty", 1)
        item_name = ITEM_MAP.get(tag)
        if not item_name:
            continue  # Skip unknown tags
        items.extend([item_name] * qty)
        keys_to_items.setdefault(item_name, []).append(key)

    df = pd.DataFrame(items, columns=["Item"])
    counts = df["Item"].value_counts().reset_index()
    counts.columns = ["Item", "Quantity"]
    counts["Unit Price (PKR)"] = counts["Item"].apply(lambda x: ITEM_PRICES.get(x, 0))
    counts["Total Price (PKR)"] = counts["Quantity"] * counts["Unit Price (PKR)"]

    total_bill = counts["Total Price (PKR)"].sum()

    return counts, len(items), keys_to_items, total_bill

def delete_one_item(item_name, keys_map):
    keys = keys_map.get(item_name, [])
    if not keys:
        return
    logs_ref.child(keys[0]).delete()

# ---------------------------
# Streamlit App UI
# ---------------------------
placeholder = st.empty()

while True:
    with placeholder.container():
        st.title("🛒 Smart Trolley Dashboard")
        st.subheader("🧾 Scanned Item Summary")
        now = datetime.now().strftime("%d-%m-%Y %H:%M:%S")
        st.markdown(f"🕒 **Current Time:** {now}")

        item_counts, total_items, keys_map, total_bill = fetch_data()

        # Custom table formatting to place values below headers
        if not item_counts.empty:
            col1, col2, col3 = st.columns([2, 1, 1])
            with col1:
                st.markdown("**Item**")
                for item in item_counts["Item"]:
                    st.markdown(f"{item}")
            with col2:
                st.markdown("**Unit Price (PKR)**")
                for price in item_counts["Unit Price (PKR)"]:
                    st.markdown(f"{price}")
            with col3:
                st.markdown("**Total Price (PKR)**")
                for tprice in item_counts["Total Price (PKR)"]:
                    st.markdown(f"{tprice}")
        else:
            st.warning("No items scanned yet.")

        # Metrics display
        col1, col2, col3, col4, col5 = st.columns(5)
        col1.metric("🍎 Apple", int(item_counts[item_counts['Item'] == "Apple"]['Quantity'].values[0]) if "Apple" in item_counts['Item'].values else 0)
        col2.metric("🥔 Lays", int(item_counts[item_counts['Item'] == "Lays"]['Quantity'].values[0]) if "Lays" in item_counts['Item'].values else 0)
        col3.metric("🍪 Biscuit", int(item_counts[item_counts['Item'] == "Biscuit"]['Quantity'].values[0]) if "Biscuit" in item_counts['Item'].values else 0)
        col4.metric("🍦 Ice Cream", int(item_counts[item_counts['Item'] == "Ice cream"]['Quantity'].values[0]) if "Ice cream" in item_counts['Item'].values else 0)
        col5.metric("🥛 Milk", int(item_counts[item_counts['Item'] == "Milk"]['Quantity'].values[0]) if "Milk" in item_counts['Item'].values else 0)

        # 🗑️ Remove items
        st.markdown("### 🗑️ Remove an Item")
        btn_col1, btn_col2, btn_col3, btn_col4, btn_col5 = st.columns(5)

        if btn_col1.button("Remove Apple"):
            delete_one_item("Apple", keys_map)
            st.success("Removed one Apple")

        if btn_col2.button("Remove Lays"):
            delete_one_item("Lays", keys_map)
            st.success("Removed one Lays")

        if btn_col3.button("Remove Biscuit"):
            delete_one_item("Biscuit", keys_map)
            st.success("Removed one Biscuit")

        if btn_col4.button("Remove Ice Cream"):
            delete_one_item("Ice cream", keys_map)
            st.success("Removed one Ice Cream")

        if btn_col5.button("Remove Milk"):
            delete_one_item("Milk", keys_map)
            st.success("Removed one Milk")

        # Summary
        st.divider()
        st.metric("📦 Total Items", total_items)
        st.metric("💰 Total Bill (PKR)", f"Rs. {total_bill}")
        st.info(f"Auto-refreshing every {refresh_interval} seconds...")

        st.markdown("---")
        st.markdown("### 👨‍💻 Developed By:")
        st.markdown("1. **Ali Muneer**  \n2. **Mohib Hassan**  \n3. **Muhammad Ikram**")
        st.markdown("### 👨‍🏫 Supervised By:\n**Dr. Abdur Raheem**")

    time.sleep(refresh_interval)
    st.rerun()
